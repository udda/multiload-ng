## Ideas for new data providers

This document contains some ideas and suggestions about new data providers.

Some of them are hard/impossible to implement, others are just sitting there,
waiting for some programmers spare time.


### hddtemp

Difficulty: simple

A simple TCP client that connects to a `localhost` socket (default port
`7634`, user-configurable).  
Server returns a single line with hard disk temperature data (if any), separated
by a field separator (default separator `|`, user-configurable)


### S.M.A.R.T

Difficulty: simple

Should be easy, because there are some ready to use libraries (like
**libatasmart**) that return S.M.A.R.T. data.


### GPU temperature (ATI, nVIDIA)

Difficulty: simple

I simply do not own any of these GPUs, so I don't know how to collect the data.
It should be very simple (maybe some sysfs path created by GPU kernel module).

Unfortunately there is not much documentation on the Internet, although a quick
workaround could be to look at the source of one of the many GPU monitor available
for Linux.


### CPU voltage

Difficulty: simple

It is sufficient to have a PC with CPU voltage sysfs support (to know which
keys to read). Reading should be the same as CPU frequency.

### Fan speed

Difficulty: simple

It is sufficient to have a PC with fan speed sysfs support (to know which
keys to read). Reading should be the same as CPU frequency.


### Bluetooth traffic

Difficulty: hard

The idea is to implement a Bluetooth graph that works exactly like Net graph.

AFAIK, Bluetooth subsystem in Linux does not expose traffic information directly.
Maybe it's necessary to do packet sniffing or something similar.


### Audio level

Difficulty: hard

The idea is to implement a sort of VU-meter as a graph.

With ALSA it's possible to measure **input** sound, by collecting sound data
and calculating peak value. This way of measuring is both inefficient and
not suitable for Multiload-ng data collect model (which does very less frequent
sampling).

Here is example code with ALSA:

	//deps: alsa-lib (libasound2 in Ubuntu)
	#include <asoundlib.h>
	#include <stdio.h>
	#include <math.h>
	static short
	GetLevel (snd_pcm_uframes_t n_samples)
	{
		int i;
		short result = 0.0f;
		snd_pcm_t* waveform;
		// Open and initialize a waveform
		if (snd_pcm_open (&waveform, "default", SND_PCM_STREAM_CAPTURE, SND_PCM_ASYNC) != 0)
			return 0;
		// Set the hardware parameters
		if (!snd_pcm_set_params (waveform, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 2, 44100, 1, 0)) {
			// Read current samples
			short buffer[2*n_samples];
			if (snd_pcm_readi (waveform, buffer, n_samples) == n_samples) {
				// Compute the maximum peak value
				for (i = 0; i < 2*n_samples; i++) {
					// Substitute better algorithm here if needed
					short s = buffer[i];
					if (s < 0)
						s = -s;
					if (result < s)
						result = s;
				}
			}
		}
		snd_pcm_close (waveform);
		return result;
	}
	int
	main()
	{
		// NOTE: this is input level! For output, an audio loopback device must be created
		while(1) {
			short level = GetLevel(22050);
			int log = 20*log10(level);
			printf("% 8d | % 8d | ", level, log);
			for (; log>1; log--) {
				printf("-");
			}
			printf("|\n");
			sleep(1);
		}
		return 0;
	}


In addition, I think it's way more useful/intuitive to measure **output** sound,
which is impossible with ALSA without custom system configuration (that should
be avoided at any cost, Multiload-ng should work out-of-the-box),

Maybe PulseAudio might help with that (could be an optional dependency, like
JavaScript).


### Weather

Difficulty: ???

Is there a weather parameter that changes so fast to be drawn in a graph?
If not, this graph is simply not feasible.

Also, it's necessary a free weather API service with unlimited queries.

Once such API is available, it's only a matter of parsing JSON/XML data (easy).


### Finance / stock exchange

Difficulty: ???

It's necessary to find a free stock exchange API service with unlimited queries.

Once such API is available, it's only a matter of parsing JSON/XML data (easy).


### Keypresses, mouse speed

Difficulty: hard

The real issue there is to measure the number of keystrokes and the mouse speed:

- without using X (that would make it impossible to implement the headless server)
- without root


### Number of LAN hosts

Difficulty: hard

The idea is to count hosts in the current LAN, possibly with some filter.

The main requisite is that these checks have to be done **without root**.


### Online stats

Difficulty: ???

The idea is to gather some statistics from a website, if any.
Some examples may include:

- online Twitter users
- Google searches per second
- online players in some MMORPG


### Open browser tabs

Difficulty: hard

The main problem is to retrieve number of open tabs without any modification
to the browser (plugins, extensions, ...).

Often tabs are stored in some format (JSON, MySQL) together with other data
that could be collected too.


### Other sensors

Difficulty: average

The idea is to extend hwmon interface designed for temperature graph, to make
it recognize and parse other sensors too (this may include fans, and custom
sensors in some laptops).


### File count in directory

Difficulty: very easy

This should be pretty straightforward to implement. The idea is to count files
into a selected directory, maybe with a filter.

Here are some additional features:

- choice between recursive/non recursive search
- pattern (regex) to select files
- choice between files, directories, or both
- choice between follow symlinks or not

Example code for listing directory contents:

	int file_count = 0;
	DIR * dirp;
	struct dirent * entry;

	dirp = opendir("path"); /* There should be error handling after this */
	while ((entry = readdir(dirp)) != NULL) {
		if (entry->d_type == DT_REG) { /* If the entry is a regular file */
			file_count++;
		}
	}
	closedir(dirp);


### Scriptable graphs

Difficulty: hard

The idea is to embed interpreters for scripting languages, in order to allow users
to write custom graph in their favorite language, much like it's already done with
JavaScript.

Here are some languages that can be embedded in C:

- **Lua** (like Conky), through `liblua` (see [here](https://www.lua.org/pil/24.html))
- **Python** (see [here](https://docs.python.org/2/extending/embedding.html))
- **Perl** (see [here](http://perldoc.perl.org/perlembed.html) or [here](http://docstore.mik.ua/orelly/perl/prog3/ch21_04.htm))
- **TCL** (see [here](http://wiki.tcl.tk/2074))
- **Scheme** (see [here](https://pubby8.wordpress.com/2012/03/22/scheme-with-c/))
- **Squirrel** (see [here](http://wiki.squirrel-lang.org/default.aspx/SquirrelWiki/Embedding%20Getting%20Started.html))
- **C** (see below)

I am also thinking of a C-to-C provider, that could be used by developers
that are embedding Multiload-ng into their application, to feed a custom graph
without the overhead (and learning curve) of a scripting language.  
This (yet to be named) provider could be implemented with a public function like `multiload_c_graph_push_data ()`.

Maybe some of the above languages are not practical to create a custom graph, I
just collected here all that I found.

Each provider should have access to the same information available to standard
providers (limits, first_call, and so on).
Each provider should have detailed documentation of available functions
to interfacing with Multiload-ng.

The last thing to check is the license. If the interpreter is not a library
(code to directly include into main tree), its license must be compatible
with `libmultiload`'s *GPLv3*.  
For example, Duktape JS engine is licensed under MIT (that is compatible with almost everything).

