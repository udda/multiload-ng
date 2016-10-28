# Multiload-ng Changelog

This document groups together changelogs for every Multiload-ng release.

- [Multiload-ng v1.4.2](#multiload-ng-v142)
- [Multiload-ng v1.4.1](#multiload-ng-v141)
- [Multiload-ng v1.4.0](#multiload-ng-v140)
- [Multiload-ng v1.3.1](#multiload-ng-v131)
- [Multiload-ng v1.3.0](#multiload-ng-v130)
- [Multiload-ng v1.2.0](#multiload-ng-v120)
- [Multiload-ng v1.1.0](#multiload-ng-v110)
- [Multiload-ng v1.0.1](#multiload-ng-v101)
- [Multiload-ng v1.0](#multiload-ng-v10)










## Multiload-ng v1.4.2

### Changelog
- Compatibility with Ubuntu 12.04 and newer (thanks to @spvkgn)

### Notes about upgrade
This is a minor release. When upgrading from previous major releases, it is really recommended to read [release notes of Multiload-ng 1.4.0](#multiload-ng-v140).










## Multiload-ng v1.4.1

### Bugfixes
- Fixed: MATE applet won't start due to missing GSettings keys (thanks to @hotice)

### Notes about upgrade
This is a minor release. When upgrading from previous major releases, it is really recommended to read [release notes of Multiload-ng 1.4.0](#multiload-ng-v140).










## Multiload-ng v1.4.0

### Changelog
- Support for Ubuntu Unity and others (AppIndicator plugin)
- Support for System tray
- Package generators for most Linux distros
- Ability to switch between SI units (base 1000) and IEC units (base 1024) to measure bytes
- Drop *shared* component of Memory graph
- Ability to choose between two methods of counting used memory
- Command line options parsing
- Simulate *panel orientation* on standalone, based on width-to-height ratio
- Ability to import color schemes created with older versions of Multiload-ng
- Ability to choose background gradient direction
- Better graphs drawing performance
- Better portability of build scripts
- Other minor improvements

### Bugfixes
- Fixed: preferences window crashed when running with older GTK+3 versions (<3.16)
- Fixed: graphs drawed a line over the border when the autoscaler maximum was 0 (empty data for long time)
- Fixed: parametric command tester reported empty command lines as *invalid*
- Fixed: Memory simple tooltip reported used+cached RAM instead of just used RAM
- Other minor fixes

### Special thanks
Thanks to [Alexander](mailto:sfslinux@gmail.com) and @Photon89 for their useful suggestions and exhaustive beta tests

### Release notes

#### New plugins
There are two new plugins: **system tray** and **indicator**. Because of them making use of advanced GTK features, minimum required GTK+2 version raises to 2.20. GTK+3 can still be any version since 3.0.0.

**System tray** plugin allows Multiload-ng to be embedded in virtually *any* panel, as long as it has a system tray. It has less options than other variants of Multiload-ng, because of tray icons limitations. That aside, is a perfectly working implementation.

**Indicator** plugin adds support to Ubuntu Unity among others. Broadly speaking, this plugin can be embedded within any panel with *libappindicator* support (I'm not sure, but this should include KDE too).

#### Improvements to Memory graph
This release brings two major changes to Memory graph.

The first is that, after doing some research, **shared** memory count was removed from Multiload-ng. This is the approach followed by many resources monitor.
Why? Because it's misleading and confusing. Most of shared memory is also included in *cached* memory, and this caused over 100% memory usage in some situations due to double counting the memory. This was very noticeable when storing large files in *tmpfs* mounts.
A graph where all components are required to be disjoint sets is simply not the place for shared memory count. As result of removing shared memory, now the graph reports correct memory usage percentages.

The second change is the introduction of the ability to choose between two methods of counting used memory, depending on where **Slab** cache should be counted:
- count it in *cache* (previous behavior): tools like `free` and `top` do that.
- count it in *used*: tools like `htop` and some graphical system monitors do that.

Choosing one method or another is a matter of habit. As you can see if you try it, the total amount of memory does not change. You can use this preference to align memory values with those of your favorite tool.

#### Gradient background
This release introduces the ability to change the direction of background gradient, individually, for each graph. There are 8 possible **linear** directions (one every 45 degrees), and one **radial** direction (from center outwards)

#### Package generators
This release includes in the source package generators for several Linux distributions:
- ebuild (Gentoo, and derivatives)
- PKGBUILD (Arch and derivatives)
- deb (Debian and derivatives, Ubuntu and derivatives, Linux Mint, and others)
- RPM (Red Hat Linux, Fedora, Mandriva, SuSe, OpenSUSE, PCLinuxOS, Mageia, and others)
- Slackware

Take a look at [extras directory](https://github.com/udda/multiload-ng/tree/master/extras) and its README.

#### Command line options parsing
Plugins that are also individual applications (namely: standalone, indicator. systray) now accept also simple command line options, for example to open Preferences window on application startup.
Run application in a terminal with `--help` argument to get the full list of supported arguments.










## Multiload-ng v1.3.1

### Changelog
- Improvements to system tray plugin (still experimental, but now usable)
- Better fallback when no temperature sources are available (e.g. running in a virtual machine)
- Raised default update interval (less overhead, avoid issues with tooltips to new users)
- New color scheme: **Colored Glass** (first builtin scheme that makes use of transparency)
- Internal improvements (useful to Multiload-ng developers and debuggers)

### Bugfixes
- Setting update interval too low, pops up a warning about tooltips can't be reliably displayed. Warning level was set one step lower than needed on GTK+2 builds. Now the warning level is set to the correct value. (thanks to @hotice)

### New languages
- Simplified Chinese localization (thanks to Dee.H.Y <dongfengweixiao@hotmail.com> @dongfengweixiao)

### Notes about upgrade
This is a minor release. When upgrading from 1.2.0, it is really recommended to read [release notes of Multiload-ng 1.3.0](#multiload-ng-v130).










## Multiload-ng v1.3.0

### Changelog
- Drop libgtop dependency (less overhead and more precise measurements)
- GTK+3 plugin can be used in XFCE >= 4.12.0
- New graph: Parametric (draws output of arbitrary commands)
- More Temperature sources (using *hwmon*)
- User selectable maximum value for each graph
- User selectable graph sources
- Display "Critical" temperature too in Temperature graph, with a different color
- Show more system info in Load Average detailed tooltip
- Show "nice" and "system" CPU percentage in CPU detailed tooltip
- Additional tokens for double click command line
- Resizable Preferences Window
- New color scheme: Windows Metro
- Added icons next to color schemes
- Fine tuning of some color schemes
- Other minor improvements

### Bugfixes
- Fixed: under some circumstances, XFCE plugin did not use locale
- Fixed: background gradient sometimes had wrong height
- Fixed: Desktop files had no localization
- Fixed: under some circumstances, after panel orientation change, graphs had 1px size
- Fixed: under some circumstances, tooltips showed after too much time
- Fixed: changes in settings required a full rebuild of widget structure
- Other minor fixes

### New languages
- Lithuanian localization (thanks to Moo <hazap@hotmail.com>)

### Special thanks
Thanks to Michael Kogan (<michael.kogan@gmx.net>) and Alexander (<sfslinux@gmail.com>) for their useful suggestions and exhaustive beta tests

### Notes about upgrade
It is recommended to read the README before installing Multiload-ng.

#### Settings reset
Upgrade from a previous version to v1.3.0 will reset some settings to default value. Don't worry, this will happen ONLY the first time the plugin is launched.

#### Dropped libgtop
This release drops **libgtop** dependency. It used to cause many problems in the past, due to lack of proper documentation and sometimes incorrect reporting of system resources usage (see #10). Now system data is read directly from nodes in /proc and /sys, resulting in less memory overhead and less CPU usage. This also corrects RAM usage stats, aligning them to values returned by `free`.

#### Parametric graph
This long-awaited feature has finally come. Parametric graph executes any valid command line, and draws up to 4 float values and a one-line text message. It can be used to monitor virtually anything! Detailed instructions are in a tooltip next to Advanced settings for this graph.

#### Selectable maximum display value
Multiload-ng 1.3.0 allows user to choose max display value of every graph (except those that use percentages, whose maximum is always *100%*). Previous behavior (automatic calculation of max value) is still possible by checking the dedicate checkbox.

#### Selectable sources
Multiload-ng 1.3.0 has **filters** support: in graphs with multiple sources (namely *network*, *disk* and *temperature*) user can choose which sources use in calculations. Previous behavior (automatic selection) is still possible by checking the dedicate checkbox.









## Multiload-ng v1.2.0

### Changelog
* New preferences window
* Each graph can now have different size
* Each graph can now have different update interval
* Each graph can now have different tooltip style
* Each graph can now have different double click action
* Double click action can now parse parametric arguments (see tooltip next to command entry)
* Full color schemes support
* 18 builtin color schemes
* Fixed temperature graph behavior under some circumstances
* Fixed LxPanel sometimes not saving settings (see #9 )
* Improvement to build system
* Minor UI improvements
* Other fixes

### New languages
* French localization (thanks to Roberta Assennato <r.assennato@gmail.com>)
* German localization (thanks to Michael Kogan <michael.kogan@gmx.net>)
* Russian localization (thanks to Alexander <sfslinux@gmail.com>)

### Notes about upgrade
Upgrade from a previous version to v1.2.0 will reset some settings to default value. Don't worry, this will happen ONLY the first time the plugin is launched.

Files related to Multiload-ng are now located in directory `$HOME/.config/multiload-ng/`. This includes user color schemes and some setting files. You may have to remove old settings files in `$HOME/.config/` by hand. Or you can just ignore this message: these leftover files do not cause any harm at all.










## Multiload-ng v1.1.0

Changelog:
* Full GTK3 support
* Build system improvements
* Other fixes










## Multiload-ng v1.0.1

Changelog:
* Successful build on Debian / *buntu
* Fixed some gcc warnings
* Other fixes










## Multiload-ng v1.0

First stable release of Multiload-ng.

Warning: this fails to build on Debian/Ubuntu.