# Initialization
AC_PREREQ(2.60)
AC_INIT([multiload-ng], [1.2.0], [mr.udda@gmail.com])
AC_SUBST(APPICON, utilities-system-monitor)
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([1.11 -Wall -Werror no-dist-gzip dist-xz silent-rules foreign])
AM_SILENT_RULES([yes])
AM_MAINTAINER_MODE()


# Checks for programs.
AM_PROG_AR()
LT_PREREQ([2.2.6])
LT_INIT([disable-static])
AC_PROG_CC()
# Compiling sources with per-target flags requires AM_PROG_CC_C_O
AM_PROG_CC_C_O()
AC_PROG_LD()
AC_PROG_INSTALL()
AC_PROG_LIBTOOL()
AC_PROG_INTLTOOL(, [no-xml])


# Check for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([time.h glib.h math.h dirent.h mntent.h ctype.h stdio.h errno.h])


# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
# -Wall
AC_MSG_CHECKING(whether compiler understands -Wall)
old_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Wall"
AC_TRY_COMPILE([],[], AC_MSG_RESULT(yes), AC_MSG_RESULT(no); CFLAGS="$old_CFLAGS")


# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset rint strchr strncasecmp strtol])


# I18n
rm -f $srcdir/po/LINGUAS
for po_file in `ls $srcdir/po/*.po | sort`; do
	echo "$po_file" | sed "s|.*/po/\(.*\)\.po|\1|g" >> $srcdir/po/LINGUAS
done
# Set up gettext
GETTEXT_PACKAGE=multiload-ng
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package.])
AM_GLIB_GNU_GETTEXT


# Check highest available GTK version
PKG_CHECK_EXISTS([gtk+-3.0], [auto_gtk=3.0], [auto_gtk=2.0])
# Select GTK version
AC_MSG_CHECKING([which gtk+ version to compile against])
AC_ARG_WITH([gtk], [AS_HELP_STRING([--with-gtk=2.0|3.0|auto], [which GTK+ version to compile against (default: auto)])],
	[case "$with_gtk" in
		2.0|3.0) ;;
		auto) with_gtk=$auto_gtk ;;
		*) AC_MSG_ERROR([invalid gtk version specified]) ;;
	esac],
	[ with_gtk=$auto_gtk ]
)
AC_MSG_RESULT([$with_gtk])

case "$with_gtk" in
	2.0)
		GTK_API=2
		GTK_REQUIRED=2.18.0
		;;
	3.0)
		GTK_API=3
		GTK_REQUIRED=3.0.0
		;;
esac
AC_SUBST(GTK_API_VERSION)
AC_DEFINE_UNQUOTED(GTK_API, $GTK_API, [GTK API version to compile against.])


PKG_CHECK_MODULES(GTK, gtk+-$GTK_API.0 >= $GTK_REQUIRED)
AC_SUBST(GTK_FLAGS)
AC_SUBST(GTK_LIBS)

# Other dependencies
PKG_CHECK_MODULES(CAIRO, [cairo >= 1.0])
AC_SUBST(CAIRO_CFLAGS)
AC_SUBST(CAIRO_LIBS)


# Disable Glib deprecation warnings
AC_ARG_ENABLE([deprecations], AS_HELP_STRING([--disable-deprecations], [Disable GDK/GTK deprecation warnings (useful when compiling with -Werror)]),
	[if test "x$enableval" = "xno"; then
		AC_DEFINE([GDK_DISABLE_DEPRECATION_WARNINGS], [1], [Disable GDK/GTK deprecation warnings])
	fi]
)

# Experimental code blocks
AC_ARG_ENABLE([experimental], AS_HELP_STRING([--enable-experimental], [Compile also experimental code blocks. Warning: this is often unstable. Enable only if you are developing new features enclosed between #ifdef MULTILOAD_EXPERIMENTAL.]),
	[if test "x$enableval" = "xyes"; then
		AC_DEFINE([MULTILOAD_EXPERIMENTAL], [1], [Enable testing code (unstable)])
	fi]
)

# Developer mode (additional checks/logs)
AC_ARG_ENABLE([developer-mode], AS_HELP_STRING([--enable-developer-mode], [Enable additional checks/logs/internal stuff useful to plugin developers. Do not enable this.]),
	[if test "x$enableval" = "xyes"; then
		AC_DEFINE([MULTILOAD_DEVELOPER_MODE], [1], [Enable developer mode])
	fi]
)

# Enable debug CFLAGS
AC_ARG_ENABLE([debug], AS_HELP_STRING([--enable-debug], [Enable CFLAGS for debugging with GDB.]),
	[if test "x$enableval" = "xyes"; then
		CFLAGS+=" -ggdb -O0"
	fi]
)

# Enable profiling CFLAGS
AC_ARG_ENABLE([profiling], AS_HELP_STRING([--enable-profiling], [Enable CFLAGS for profiling with gprof.]),
	[if test "x$enableval" = "xyes"; then
		CFLAGS+=" -pg"
	fi]
)


# Per-panel configure
m4_include([data/configure.m4])
m4_include([standalone/configure.m4])
m4_include([indicator/configure.m4])
m4_include([lxpanel/configure.m4])
m4_include([mate/configure.m4])
m4_include([xfce4/configure.m4])


# Checks if at least one build is available
if \
	test x$with_standalone != xyes && \
	test x$with_indicator != xyes && \
	test x$with_lxpanel != xyes && \
	test x$with_mate != xyes && \
	test x$with_xfce4 != xyes
then
	AC_MSG_FAILURE([No supported panels are installed and available.])
fi


# Directories; for substitution into .desktop files.
# From https://mail.gnome.org/archives/commits-list/2012-March/msg14085.html
# Of course it's worth noting that the patch in that message was to replace
# this nasty hack with an awk version that runs from the Makefile...
if test "${prefix}" = "NONE"; then
	prefix=/usr/local
fi
if test "${exec_prefix}" = "NONE"; then
	exec_prefix=$prefix
fi
eval LIBDIR="$libdir"
AC_SUBST([LIBDIR])

# UI directory (Glade, colors...)
eval UIDIR=$datadir/multiload-ng
AC_SUBST([UIDIR])

# Output files
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
	Makefile
	common/Makefile
	po/Makefile.in
])
AC_OUTPUT


echo
echo
echo "* ----------------------"
echo "*  Build Configuration"
echo "* ----------------------"
echo "*"
echo "*  CFLAGS: $CFLAGS"
if test "x$enable_debug" = "xyes"; then
echo "*  Enabled debugging CFLAGS"
fi
if test "x$enable_profiling" = "xyes"; then
echo "*  Enabled profiling CFLAGS"
fi
if test "x$enable_deprecations" = "xno"; then
echo "*"
echo "*  Disabled GDK/GTK deprecation warnings"
fi
if test "x$enable_experimental" = "xyes"; then
echo "*"
echo "*  Enabled experimental code (WARNING: this is unstable)"
fi
echo "*"
if test "x$with_gtk" = "xauto"; then
echo "*  Building against GTK+$GTK_API (detected automatically)"
echo "*  Please make sure that your panel supports GTK+$GTK_API plugins."
else
echo "*  Building against GTK+$GTK_API (set by command line option)"
fi
echo "*"
echo "*  Enabled plugins:"
echo "*   - standalone  : $with_standalone"
echo "*   - indicator   : $with_indicator"
echo "*   - lxpanel     : $with_lxpanel"
echo "*   - mate-panel  : $with_mate"
echo "*   - xfce4-panel : $with_xfce4"
echo "*"
echo "* ----------------------"
echo
