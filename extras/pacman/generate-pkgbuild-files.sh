#!/usr/bin/env bash
#
# Copyright (C) 2016 Mario Cianciolo <mr.udda@gmail.com>
#
# This file is part of multiload-ng.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#

pkgver=$(cat ../../version)
pkgrel=1
md5sums="NONE"

value_in_array()
{
	local needle="$1"
	shift 1

	for i in $*; do
		if [ "$needle" = "$i" ]
			# found!
			then return 0;
		fi
	done

	# not found
	return 1
}

gen_md5sum()
{
	local tmpfile=$(mktemp)

	printf -- "Generating MD5 sums ... " >&2

	if ! which wget >/dev/null 2>&1 ; then
		echo SKIP
		printf -- "SKIP: wget not found\n" >&2
	elif ! wget -q "https://github.com/udda/multiload-ng/archive/v$pkgver.tar.gz" -O "$tmpfile" ; then
		echo SKIP
		printf -- "SKIP: could not download tarball\n" >&2
	else
		md5sum $tmpfile | while read -a array; do echo ${array[0]} ; done
		printf -- "OK\n" >&2
	fi

	rm "$tmpfile" >&2
}

get_pkgname()
{
	case "$1" in
		awn)		echo 'awn-applet-multiload-ng' ;;
		indicator)	echo 'multiload-ng-indicator' ;;
		lxpanel)	echo 'lxpanel-multiload-ng-plugin' ;;
		mate)		echo 'mate-multiload-ng-applet' ;;
		standalone)	echo 'multiload-ng-standalone' ;;
		systray)	echo 'multiload-ng-systray' ;;
		xfce4)		echo 'xfce4-multiload-ng-plugin' ;;
	esac
}

get_pkgdesc()
{
	case "$1" in
		awn)		echo 'Modern graphical system monitor, Avant Window Navigator applet' ;;
		indicator)	echo 'Modern graphical system monitor, AppIndicator plugin' ;;
		lxpanel)	echo 'Modern graphical system monitor, LxPanel plugin' ;;
		mate)		echo 'Modern graphical system monitor, MATE panel applet' ;;
		standalone)	echo 'Modern graphical system monitor, standalone version' ;;
		systray)	echo 'Modern graphical system monitor, system tray version' ;;
		xfce4)		echo 'Modern graphical system monitor, XFCE4 panel plugin' ;;
	esac
}

get_depends()
{
	if [ "$2" = "gtk2" ]; then
		printf -- "'gtk2>=2.20.0' 'cairo'"
		case "$1" in
			awn)		printf -- " 'avant-window-navigator>=0.3.9' 'glibmm>=2.16.0' 'gtkmm>=2.20'" ;;
			indicator)	printf -- " 'libappindicator-gtk2>=0.4.92'" ;;
			lxpanel)	printf -- " 'lxpanel>=0.5.8'" ;;
			mate)		printf -- " 'mate-panel>=1.7.0'" ;;
			standalone)	return 0 ;;
			systray)	return 0 ;;
			xfce4)		printf -- " 'libxfce4util>=4.6.0' 'xfce4-panel>=4.6.0'" ;;
		esac
	else
		printf -- "'gtk3' 'cairo'"
		case "$1" in
			awn)		return 1 ;;
			indicator)	printf -- " 'libappindicator-gtk3>=0.4.92'" ;;
			lxpanel)	printf -- " 'lxpanel-gtk3>=0.5.8'" ;;
			mate)		printf -- " 'mate-panel-gtk3>=1.7.0'" ;;
			standalone)	return 0 ;;
			systray)	return 0 ;;
			xfce4)		printf -- " 'libxfce4util>=4.12.0' 'xfce4-panel>=4.12.0'" ;;
		esac
	fi
}

get_replaces()
{
	if [ "$1" = "xfce4" -a "$2" = "gtk2" ]; then
		printf -- "'xfce4-multiload-ng-plugin'"
	fi
}

get_configure_string()
{
	printf -- '--prefix=/usr'

	if [ "$2" = "gtk2" ];
		then printf -- ' --with-gtk=2.0'
		else printf -- ' --with-gtk=3.0 --disable-deprecations'
	fi

	if [ "$1" = "awn" ]
		then printf -- ' --with-awn --enable-experimental'
		else printf -- ' --without-awn'
	fi
	if [ "$1" = "indicator" ]
		then printf -- ' --with-indicator'
		else printf -- ' --without-indicator'
	fi
	if [ "$1" = "lxpanel" ]
		then printf -- ' --with-lxpanel'
		else printf -- ' --without-lxpanel'
	fi
	if [ "$1" = "mate" ]
		then printf -- ' --with-mate'
		else printf -- ' --without-mate'
	fi
	if [ "$1" = "standalone" ]
		then printf -- ' --with-standalone'
		else printf -- ' --without-standalone'
	fi
	if [ "$1" = "systray" ]
		then printf -- ' --with-systray'
		else printf -- ' --without-systray'
	fi
	if [ "$1" = "xfce4" ]
		then printf -- ' --with-xfce4'
		else printf -- ' --without-xfce4'
	fi
}

generate_pkgbuild()
{
	local is_git=''
	local gtk_str='gtk2'
	local target=''

	for arg in "$@"; do
		case "$arg" in
			git)
				is_git=1 ;;
			gtk2 | gtk3)
				gtk_str="$arg" ;;
			awn | indicator | lxpanel | mate | standalone | systray | xfce4)
				target="$arg" ;;
		esac
	done


	# sanity check
	if [ -z "$target" ]; then
		echo "ERROR: no target specified" >&2
		return 1
	fi

	# generate MD5 sum from downloaded tarball
	[ "$md5sums" = "NONE" ] && md5sums=$(gen_md5sum)

	# there is no GTK3 support in AWN
	[ "$target" = "awn" -a "$gtk_str" = "gtk3" ] && return 2


	# additional parsing
	if [ -n "$is_git" ]; then
		local pkgver=`printf -- "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"`
		local version_str="git (${pkgver})"
		local git_suffix='-git'
		local makedepends="'intltool' 'git'"
		local pkg_source="git+https://github.com/udda/multiload-ng.git"
		local pkg_md5sum='SKIP'
		local pkg_basedir='multiload-ng'
	else
		local version_str="$pkgver"
		local git_suffix=''
		local makedepends="'intltool'"
		local pkg_source="https://github.com/udda/multiload-ng/archive/v\$pkgver.tar.gz"
		local pkg_md5sum="$md5sums"
		local pkg_basedir='multiload-ng-$pkgver'
	fi

	local _p="$(get_pkgname $target)"

	local pkgname="${_p}-${gtk_str}${git_suffix}"
	local pkgdesc="$(get_pkgdesc $target)"
	local depends="$(get_depends $target $gtk_str)"
	local replaces="$(get_replaces $target $gtk_str)"
	local configure_opts="$(get_configure_string $target $gtk_str)"

	# conflicts
	local conflicts=''
	for i in "${_p}-gtk2" "${_p}-gtk3" "${_p}-gtk2-git" "${_p}-gtk3-git"; do
		if [ ! "${pkgname}" = "$i" ]; then
			# appending with echo eliminates extra whitespaces
			conflicts=`echo ${conflicts} "'${i}'"`
		fi
	done

	# output
	printf -- "Package:     (target: %-13s version: %-21s $gtk_str)\n" "$target" "$version_str" >&2

	printf -- "Generating PKGBUILD ... " >&2
	local outdir="multiload-ng.PKGBUILD/${pkgname}"
	mkdir -p "${outdir}"

	cat >"${outdir}/PKGBUILD" <<-EOF
		# Maintainer: Mario Cianciolo <mr.udda at gmail dot com>
		# Co-maintainer: Michael Kogan <michael.kogan at gmx dot net>

		# This file is automatically generated from multiload-ng source.

		pkgname='${pkgname}'
		pkgdesc='${pkgdesc}'
		pkgver=${pkgver}
		pkgrel=${pkgrel}

		makedepends=(${makedepends})
		depends=(${depends})

		conflicts=(${conflicts})

	EOF

	[ -n "$replaces" ] && printf -- "replaces=(${replaces})\n\n" >>"${outdir}/PKGBUILD"

	cat >>"${outdir}/PKGBUILD" <<-EOF
		source=("${pkg_source}")
		md5sums=('${pkg_md5sum}')

		arch=('i686' 'x86_64')
		url='https://udda.github.io/multiload-ng/'
		license=('GPL2')

		build() {
		    cd "${pkg_basedir}"
		    ./autogen.sh
		    ./configure ${configure_opts}
		    make
		} 

		package() {
		    cd "${pkg_basedir}"
		    make DESTDIR="\$pkgdir" install
		}
	EOF

	[ -n "$is_git" ] &&	cat >>"${outdir}/PKGBUILD" <<-EOF

		pkgver() {
		    cd "${pkg_basedir}"
		    printf -- "r%s.%s" "\$(git rev-list --count HEAD)" "\$(git rev-parse --short HEAD)"
		}
	EOF
	printf -- 'OK\n' >&2


	if [ "${GENERATE_SRCINFO}" = "1" ] ; then
		printf -- "Generating .SRCINFO ... " >&2

		local T=$'\t'
		cat >"${outdir}/.SRCINFO" <<-EOF
			pkgbase = ${pkgname}
			${T}pkgdesc = ${pkgdesc}
			${T}pkgver = ${pkgver}
			${T}pkgrel = ${pkgrel}
			${T}url = https://udda.github.io/multiload-ng/
			${T}arch = i686
			${T}arch = x86_64
			${T}license = GPL2
		EOF
		unset T

		for i in ${makedepends}; do
			printf "\tmakedepends = %s\n" `echo $i | sed "s/'/ /g"` >>"${outdir}/.SRCINFO"
		done

		for i in ${depends}; do
			printf "\tdepends = %s\n" `echo $i | sed "s/'/ /g"` >>"${outdir}/.SRCINFO"
		done

		for i in ${conflicts}; do
			printf "\tconflicts = %s\n" `echo $i | sed "s/'/ /g"` >>"${outdir}/.SRCINFO"
		done

		printf "\tsource = %s\n" `eval echo ${pkg_source}` >>"${outdir}/.SRCINFO"
		printf "\tmd5sums = ${pkg_md5sum}\n" >>"${outdir}/.SRCINFO"

		printf "\npkgname = ${pkgname}\n\n" >>"${outdir}/.SRCINFO"

		printf -- 'OK\n' >&2
	fi

	printf -- '\n' >&2
}

help()
{
	cat >&2 <<-EOF
	USAGE: $0 [OPTIONS] [PKGSPEC]

	OPTIONS:
	  -s    Generate .SRCINFO next to each PKGBUILD
	  -h    Show this help


	PKGSPEC:
	  Options can be followed by exactly THREE arguments, in the following order:
	      PKG_TARGET   GTK_VERSION   PKG_VERSION
	  If not specified, this script will generate automatically all PKGBUILDs.

	PKG_TARGET: one of the following keywords:
	  awn           Generate PKGBUILD for Avant Window Navigator applet
	  indicator     Generate PKGBUILD for Indicator plugin
	  lxpanel       Generate PKGBUILD for LxPanel plugin
	  mate          Generate PKGBUILD for MATE panel applet
	  standalone    Generate PKGBUILD for Standalone application
	  systray       Generate PKGBUILD for system tray plugin
	  xfce4         Generate PKGBUILD for XFCE4 panel plugin

	GTK_VERSION: one of the following keywords:
	  gtk2          Generate PKGBUILD for GTK+2 version of the plugin
	  gtk3          Generate PKGBUILD for GTK+3 version of the plugin

	PKG_VERSION: one of the following keywords:
	  stable        Generate PKGBUILD that builds from release code (v$pkgver)
	  git           Generate PKGBUILD that builds from lastest git code
	EOF
}







# Option parsing
while getopts ':sh' FLAG; do
	case $FLAG in
		s)		GENERATE_SRCINFO=1 ;;
		h)		help ; exit 0 ;;

		\?)		echo "Invalid option: -$OPTARG" >&2
				echo "Run $0 -h for help." >&2
				exit 1 ;;
	esac
done
shift $((OPTIND-1))


# Sanity check
if [ ! "$#" = "0" -a ! "$#" = "3" ]; then
	echo "Invalid number of arguments. PKGSPEC requires exactly three arguments." >&2
	echo "Run $0 -h for help." >&2
	exit 1
fi


# Everything is OK - let's proceed
if [ "$#" = "3" ]; then

	echo "Found PKGSPEC!" >&2

	# check PKG_TARGET
	echo "- PKG_TARGET:  '$1'" >&2
	if ! value_in_array $1 awn indicator lxpanel mate standalone systray xfce4
		then echo "Invalid PKG_TARGET. Run $0 -h for help." >&2 ; exit 1
	fi

	# check GTK_VERSION
	echo "- GTK_VERSION: '$2'" >&2
	if ! value_in_array $2 gtk2 gtk3
		then echo "Invalid GTK_VERSION. Run $0 -h for help." >&2 ; exit 1
	fi

	# check PKG_VERSION
	echo "- PKG_VERSION: '$3'" >&2
	if ! value_in_array $3 stable git
		then echo "Invalid PKG_VERSION. Run $0 -h for help." >&2 ; exit 1
	fi

	generate_pkgbuild $1 $2 $3

else

	echo "No PKGSPEC provided, generating all PKGBUILDs." >&2

	for target in awn indicator lxpanel mate standalone systray xfce4; do
		for gtk in gtk2 gtk3; do
			generate_pkgbuild ${gtk} ${target}
			generate_pkgbuild ${gtk} ${target} git
		done
	done

fi
