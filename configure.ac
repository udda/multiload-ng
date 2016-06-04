# Initialization
AC_PREREQ(2.60)
AC_INIT([multiload-ng], [1.0], [mr.udda@gmail.com])
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
AC_CHECK_HEADERS([time.h glib.h math.h dirent.h stdio.h errno.h])


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


# Dependencies
AM_PATH_GTK_2_0([2.18.0],,AC_MSG_ERROR([Gtk+ 2.18.0 or higher required.]))
PKG_CHECK_MODULES(CAIRO, [cairo])
AC_SUBST(CAIRO_CFLAGS)
AC_SUBST(CAIRO_LIBS)
PKG_CHECK_MODULES(LIBGTOP, [libgtop-2.0 >= 2.11.92])
AC_SUBST(LIBGTOP_CFLAGS)
AC_SUBST(LIBGTOP_LIBS)


# Xfce
AC_ARG_WITH([xfce4], AS_HELP_STRING([--with-xfce4], [Build plugin for xfce4-panel]),
            [], [with_xfce4=check])
AS_IF([test "x$with_xfce4" != "xno"], [
    pkg_modules="libxfce4util-1.0 >= 4.6.0
                 libxfce4panel-1.0 >= 4.6.0"
    PKG_CHECK_MODULES(XFCE4, [$pkg_modules], [
        AC_DEFINE([HAVE_XFCE4], [1], [Support for xfce4-panel])
        with_xfce4=yes
        # Check if we have libxfce4ui or libxfcegui4
        PKG_CHECK_MODULES([XFCE4UI], [libxfce4ui-1 >= 4.8.0],
            [
                AC_DEFINE([HAVE_XFCE4UI], [1], [Have libxfce4ui])
                xfce_is_4_6=no
            ],[
                PKG_CHECK_MODULES([XFCEGUI4], [libxfcegui4-1.0 >= 4.6.0],
                    [AC_DEFINE([HAVE_XFCEGUI4], [1], [Have libxfcegui4])])
                xfce_is_4_6=yes
            ])
    ],[
        if test "x$with_xfce4" != xcheck; then
          AC_MSG_FAILURE(
           [--with-xfce4 was given, but test failed])
        fi
        with_xfce4=no
    ])
])
AM_CONDITIONAL([HAVE_XFCE4], [test x$with_xfce4 = xyes])
AM_CONDITIONAL([XFCE_IS_4_6], [test x$xfce_is_4_6 = xyes])
AC_SUBST(XFCE4_CFLAGS)
AC_SUBST(XFCE4_LIBS)
AC_SUBST(XFCE4UI_CFLAGS)
AC_SUBST(XFCE4UI_LIBS)
AC_SUBST(XFCEGUI4_CFLAGS)
AC_SUBST(XFCEGUI4_LIBS)


# lxpanel
AC_ARG_WITH([lxpanel], AS_HELP_STRING([--with-lxpanel], [Build plugin for lxpanel]),
            [], [with_lxpanel=check])
AS_IF([test "x$with_lxpanel" != "xno"], [
    pkg_modules="lxpanel"
    # libmenu-cache is implicitly required; we will fail if it's not present.
    PKG_CHECK_MODULES(LXPANEL, [$pkg_modules],
                      [
                        AC_DEFINE([HAVE_LXPANEL], [1], [Support for lxpanel])
                        with_lxpanel=yes
                      ],[
                        if test "x$with_lxpanel" != xcheck; then
                          AC_MSG_FAILURE(
                           [--with-lxpanel was given, but test failed])
                        fi
                        with_lxpanel=no
                      ])
])
AM_CONDITIONAL(HAVE_LXPANEL, test x$with_lxpanel = xyes)
AC_SUBST(LXPANEL_CFLAGS)
AC_SUBST(LXPANEL_LIBS)


# mate
AC_ARG_WITH([mate], AS_HELP_STRING([--with-mate], [Build plugin for mate-panel]),
            [], [with_mate=check])
AS_IF([test "x$with_mate" != "xno"], [
    GLIB_GSETTINGS
    pkg_modules="libmatepanelapplet-4.0 >= 1.7.0"
    PKG_CHECK_MODULES(MATE, [$pkg_modules],
                      [
                        AC_DEFINE([HAVE_MATE], [1], [Support for mate-panel])
                        with_mate=yes
                      ],[
                        if test "x$with_mate" != xcheck; then
                          AC_MSG_FAILURE(
                           [--with-mate was given, but test failed])
                        fi
                        with_mate=no
                      ])
])
AM_CONDITIONAL(HAVE_MATE, test x$with_mate = xyes)
AC_SUBST(MATE_CFLAGS)
AC_SUBST(MATE_LIBS)


# standalone
AC_ARG_WITH([standalone], AS_HELP_STRING([--with-standalone], [Build standalone application]),
            [], [with_standalone=yes])
AM_CONDITIONAL(HAVE_STANDALONE, test x$with_standalone != xno)


# Checks if at least one build is available
if \
    test x$with_xfce4 != xyes && \
    test x$with_lxpanel != xyes && \
    test x$with_mate != xyes && \
    test x$with_standalone != xyes
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


# Output files
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
    Makefile
    generate-about-data.sh
    common/Makefile
    lxpanel/Makefile
    mate/Makefile
    mate/org.mate.multiload-ng.Applet.mate-panel-applet.in.in
    po/Makefile.in
    standalone/Makefile
    xfce4/Makefile
    xfce4/multiload-ng.desktop.in
])
AC_OUTPUT


echo
echo "Build Configuration:"
echo
echo "* xfce4-panel :    $with_xfce4"
echo
echo "* lxpanel     :    $with_lxpanel"
echo
echo "* mate-panel  :    $with_mate"
echo
echo "* standalone  :    $with_standalone"
echo
