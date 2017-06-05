/*
 * Copyright (C) 2017 Mario Cianciolo <mr.udda@gmail.com>
 *
 * This file is part of Multiload-ng.
 *
 * Multiload-ng is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Multiload-ng is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Multiload-ng.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <multiload.h>


int
ml_timespec_to_milliseconds (struct timespec *tm)
{
	if_unlikely (tm == NULL)
		return 0;

	return tm->tv_sec * 1000 + tm->tv_nsec / 1000000;
}

void
ml_milliseconds_to_timespec (struct timespec *tm, int ms)
{
	if_unlikely (tm == NULL)
		return;

	tm->tv_sec = ms / 1000;
	tm->tv_nsec = (ms % 1000) * 1000000;
}

bool
ml_thread_set_priority (int priority)
{
	/* from setpriority manpage:
	 *  According to POSIX, the nice value is a per-process setting.
	 *  However, under the current Linux/NPTL implementation of POSIX
	 *  threads, the nice value is a per-thread attribute: different
	 *  threads in the same process can have different nice values.
	 *  Portable applications should avoid relying on the Linux behavior,
	 *  which may be made standards conformant in the future. */

	pid_t tid = syscall (SYS_gettid);
	return setpriority (PRIO_PROCESS, tid, priority) == 0;
}


static bool
ml_execute_cmdline_full (const char *cmdline, const char *working_dir, bool async, int timeout_ms, MlGrowBuffer *gbuf_stdout, MlGrowBuffer *gbuf_stderr, MlGrowBuffer *gbuf_report, int *exit_status)
{
	/* This function executes a new process. There are a few things to point out:
	 *
	 * - child process inherits priority (nice) of the parent. For provider threads,
	 *   this parameter is determined by ML_PROVIDER_THREAD_PRIORITY in global.h,
	 *   and propagating the priority is the intended behavior. For processes
	 *   launched from other threads, the starting priority is tipically unchanged (0).
	 *   Again, this is the intended behavior.
	 *
	 * - all grow buffers provided in arguments are rewinded at function start.
	 *   gbuf_stdout and gbuf_stderr are filled only when proces is executed (this function
	 *   returns true), while gbuf_report is filled only when the function fails, and
	 *   it contains an error message.
	 *
	 * - when executing asynchronously, report pipe needs to be closed immediately
	 *   before execl(), so if execl() fails, report_gbuf will contain no data, and
	 *   the only clue will be the return value (false). */

	enum { PIPE_READ=0, PIPE_WRITE=1 };

	int pipe_out    [2] = {-1, -1};
	int pipe_err    [2] = {-1, -1};
	int pipe_report [2] = {-1, -1};

	char buf[256];
	ssize_t s;

	// nested function (GCC extension)
	void cleanup_fd (int index)
	{
		if (pipe_out[index] != -1) {
			close (pipe_out[index]);
			pipe_out[index] = -1;
		}

		if (pipe_err[index] != -1) {
			close (pipe_err[index]);
			pipe_err[index] = -1;
		}

		if (pipe_report[index] != -1) {
			close (pipe_report[index]);
			pipe_report[index] = -1;
		}
	}

	// invalidate output buffers for async execution
	if (async) {
		gbuf_stdout = NULL;
		gbuf_stderr = NULL;
		exit_status = NULL;
	}

	// gbuf_report will never be checked for NULLness: MlGrowBuffer operations handle NULL gracefully, silently failing
	ml_grow_buffer_rewind (gbuf_report);


	// create pipes
	if (gbuf_report != NULL && pipe(pipe_report) < 0) {
		ml_grow_buffer_append_printf (gbuf_report, _("pipe(%s) failed: %s"), "report", strerror(errno));
		return false;
	}

	if (gbuf_stdout != NULL && pipe(pipe_out) < 0) {
		ml_grow_buffer_append_printf (gbuf_report, _("pipe(%s) failed: %s"), "stdout", strerror(errno));
		return false;
	}

	if (gbuf_stderr != NULL && pipe(pipe_err) < 0) {
		ml_grow_buffer_append_printf (gbuf_report, _("pipe(%s) failed: %s"), "stderr", strerror(errno));
		return false;
	}

	// let's fork
	pid_t pid = fork();

	if (pid < 0) { // error
		ml_grow_buffer_append_printf (gbuf_report, _("fork() failed: %s"), strerror(errno));
		cleanup_fd (PIPE_READ);
		cleanup_fd (PIPE_WRITE);
		return false;
	}

	if (pid > 0) { // parent process
		ml_debug ("Started new process with PID %d\n%s", (int)pid, cmdline);

		// close unused (write) pipes
		cleanup_fd (PIPE_WRITE);


		// wait for child termination
		int wstatus;
		if (timeout_ms > 0) { // positive timeout - wait for child until timeout, then kill it
			pid_t wpid;
			int remaining = timeout_ms;
			/* Newer techniques for implementing a timeout for waitpid (such as
			 * signalfd) cannot be used here, because they require altering
			 * signal mask in calling process.
			 * Being a library, no assumptions about signal handlers can be
			 * made, hence the reason for busy waiting. */
			while ((wpid = waitpid (pid, &wstatus, WNOHANG)) == 0) {
				/* Here it's a pseudo-busy waiting, with little sleeps
				 * between checks, which greatly improve performance */
				usleep ((useconds_t)1000 * ML_CHILD_SPINLOCK_INTERVAL);
				remaining -= ML_CHILD_SPINLOCK_INTERVAL;
				if (remaining < 0) {
					kill (pid, SIGKILL);
					ml_grow_buffer_append_printf (gbuf_report, _("Process has been running for too long (timeout set to %d ms)"), timeout_ms);
					cleanup_fd (PIPE_READ);
					return false;
				}
			}

			// on error, waitpid() returns -1, otherwise returns the waited PID (always positive)
			if (wpid < 0) {
				if (errno != ECHILD) { // ECHILD: the process does not exist anymore (not really an error)
					ml_grow_buffer_append_printf (gbuf_report, _("waitpid() failed: %s"), strerror(errno));
					cleanup_fd (PIPE_READ);
					return false;
				}
			}
		} else { // non positive timeout - wait for child indefinitely
			if (waitpid (pid, &wstatus, 0) < 0) {
				if (errno != ECHILD) { // The process does not exist anymore (not really an error)
					ml_grow_buffer_append_printf (gbuf_report, _("waitpid() failed: %s"), strerror(errno));
					cleanup_fd (PIPE_READ);
					return false;
				}
			}

		}

		// status reporting
		s = read (pipe_report[PIPE_READ], buf, sizeof(buf));
		if (s > 0) {
			ml_grow_buffer_append_printf (gbuf_report, _("Error from child: %s"), buf);
			cleanup_fd (PIPE_READ);
			return false;
		}

		// exit status
		if (WIFEXITED (wstatus)) {
			if (exit_status != NULL)
				*exit_status = WEXITSTATUS (wstatus);
		} else {
			if (WIFSIGNALED (wstatus))
				ml_grow_buffer_append_printf (gbuf_report, _("Child process killed by signal %ld"), (long)WTERMSIG(wstatus));
			else if (WIFSTOPPED (wstatus))
				ml_grow_buffer_append_printf (gbuf_report, _("Child process killed by signal %ld"), (long)WSTOPSIG(wstatus));
			else
				ml_grow_buffer_append_printf (gbuf_report, _("Child process terminated abnormally"));

			cleanup_fd (PIPE_READ);
			return false;
		}

		// read stdout
		if (gbuf_stdout != NULL) {
			ml_grow_buffer_rewind (gbuf_stdout);
			while ((s = read (pipe_out[PIPE_READ], buf, sizeof(buf))) > 0)
				ml_grow_buffer_append (gbuf_stdout, buf, s);
		}

		// read stderr
		if (gbuf_stderr != NULL) {
			ml_grow_buffer_rewind (gbuf_stderr);
			while ((s = read (pipe_err[PIPE_READ], buf, sizeof(buf))) > 0)
				ml_grow_buffer_append (gbuf_stderr, buf, s);
		}

		cleanup_fd (PIPE_READ);
		return true;

	} else { // child process

		// close unused (read) pipes
		cleanup_fd (PIPE_READ);

		// reset signal handlers modified by some tooklits (like GTK/QT)
		signal (SIGCHLD, SIG_DFL);
		signal (SIGINT, SIG_DFL);
		signal (SIGTERM, SIG_DFL);
		signal (SIGHUP, SIG_DFL);
		signal (SIGPIPE, SIG_DFL); // with this one, if parent exits and we write to the report FILE, child will crash

		// open a FILE for fprintf facility
		FILE *file_report = fdopen (pipe_report[PIPE_WRITE], "w");

		/* In order to launch process asynchronously, we need to make it orphan.
		 * This way parent's waitpid() can return, and child will be adopted by
		 * init, surviving parent termination */
		if (async) {
			pid_t pid2 = fork();
			if (pid2 < 0) { // error
				fprintf (file_report, _("fork() failed: %s"), strerror(errno));
				exit (1);
			} else if (pid2 > 0) {
				// close this child, the rest of this function is the "grandchild"
				exit (0);
			}
		}

		// nested functions (GCC extension)
		void dup2_or_exit (int fromfd, int tofd, char *name) {
			if (dup2 (fromfd, tofd) == tofd) {
				close (fromfd);
			} else {
				fprintf (file_report, _("dup2(%s) failed: %s"), name, strerror(errno));
				exit (1);
			}
		}

		void close_extra_fd () {
			DIR *dir = opendir("/proc/self/fd");
			if (dir == NULL)
				return ;

			int opendirfd = dirfd (dir);
			struct dirent *ent;

			while ((ent = readdir(dir)) != NULL) {
				if (ml_string_has_prefix (ent->d_name, "."))
					continue;

				int fd = atoll (ent->d_name);
				if (fd != opendirfd && fd != pipe_report[PIPE_WRITE] && fd != STDOUT_FILENO && fd != STDIN_FILENO && fd != STDERR_FILENO)
					close(fd);
			}

			closedir(dir);
		}


		// redirect stdout
		if (gbuf_stdout != NULL) {
			dup2_or_exit (pipe_out[PIPE_WRITE], STDOUT_FILENO, "stdout");
		} else {
			/* redirect stdout to /dev/null, else it will print in
			 * parent console (both share the same stdout FD). Just closing
			 * child stdout won't always work as some programs could block */
			int nullfd = open ("/dev/null", O_WRONLY);
			if (nullfd < 0) {
				fprintf (file_report, _("Can't open %s for writing: %s"), "/dev/null", strerror(errno));
				exit (1);
			}
			dup2_or_exit (nullfd, STDOUT_FILENO, "stdout -> /dev/null");
		}

		// redirect stderr
		if (gbuf_stderr != NULL) {
			dup2_or_exit (pipe_err[PIPE_WRITE], STDERR_FILENO, "stderr");
		} else {
			/* redirect stderr to /dev/null, see stdout comment above */
			int nullfd = open ("/dev/null", O_WRONLY);
			if (nullfd < 0) {
				fprintf (file_report, _("Can't open %s for writing: %s"), "/dev/null", strerror(errno));
				exit (1);
			}
			dup2_or_exit (nullfd, STDERR_FILENO, "stderr -> /dev/null");
		}

		/* redirect stdin from /dev/null, see stdout comment above */
		int nullfd = open ("/dev/null", O_RDONLY);
		if (nullfd < 0) {
			fprintf (file_report, _("Can't open %s for reading: %s"), "/dev/null", strerror(errno));
			exit (1);
		}
		dup2_or_exit (nullfd, STDIN_FILENO, "stdin -> /dev/null");


		// close remaining open file descriptors
		close_extra_fd();


		// change working directory
		if (working_dir != NULL) {
			if (chdir (working_dir) != 0) {
				fprintf (file_report, _("chdir(%s) failed: %s"), working_dir, strerror(errno));
				exit (1);
			}
		}

		// report pipe must be closed before exec, or parent will wait for child termination
		if (async)
			fclose (file_report);

		if (execl ("/bin/sh", "sh", "-c", cmdline, NULL) < 0) {
			if (!async)
				fprintf (file_report, _("execl() failed: %s"), strerror(errno));
			exit (1);
		}

		/* Program will never reach this point, but GCC would complain without
		 * an explicit return. Also, pipe_report[PIPE_WRITE], and corresponding
		 * FILE*, are never closed, so we can use it even after execl().
		 * That's not a problem, since they'll be closed anyway when child terminates. */
		return true;
	}
}

bool
ml_execute_cmdline_sync (const char *cmdline, const char *working_dir, int timeout_ms, MlGrowBuffer *gbuf_stdout, MlGrowBuffer *gbuf_stderr, MlGrowBuffer *gbuf_report, int *exit_status)
{
	return ml_execute_cmdline_full (cmdline, working_dir, false, timeout_ms, gbuf_stdout, gbuf_stderr, gbuf_report, exit_status);
}

bool
ml_execute_cmdline_async (const char *cmdline, const char *working_dir, int timeout_ms, MlGrowBuffer *gbuf_report)
{
	return ml_execute_cmdline_full (cmdline, working_dir, true, timeout_ms, NULL, NULL, gbuf_report, NULL);
}
