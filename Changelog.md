# Multiload-ng Changelog

This document groups together changelogs for every Multiload-ng release.

- [Multiload-ng v1.5.2](#multiload-ng-v152)
- [Multiload-ng v1.5.1](#multiload-ng-v151)
- [Multiload-ng v1.5.0](#multiload-ng-v150)
- [Multiload-ng v1.4.2](#multiload-ng-v142)
- [Multiload-ng v1.4.1](#multiload-ng-v141)
- [Multiload-ng v1.4.0](#multiload-ng-v140)
- [Multiload-ng v1.3.1](#multiload-ng-v131)
- [Multiload-ng v1.3.0](#multiload-ng-v130)
- [Multiload-ng v1.2.0](#multiload-ng-v120)
- [Multiload-ng v1.1.0](#multiload-ng-v110)
- [Multiload-ng v1.0.1](#multiload-ng-v101)
- [Multiload-ng v1.0](#multiload-ng-v10)










## Multiload-ng v1.5.2

<sup>*Released on 05 Dec 2016*</sup>

### Bugfixes
- Fixed: Desktop entry for Systray contained a small typo, because of which it was not assigned to the correct desktop categories in some systems
- Fixed: several memory leaks, some of these were severe enough to make Multiload-ng use huge amounts of memory after some days of continuous use (up to several hundreds MB)

### Notes about upgrade
This is a minor release. When upgrading from previous major releases, it is really recommended to read [release notes of Multiload-ng 1.5.0](#multiload-ng-v150).

Considering the positive effects brought by this release (smaller memory usage), upgrade is highly recommended.

Sorry for my absence this month, I'm pretty busy in real life. Development is definitely not over, and will resume soon.










## Multiload-ng v1.5.1

<sup>*Released on 18 Nov 2016*</sup>

### Changelog
- Indicator: move system monitor menu item to the top (thanks to @leoheck)
- Change *Capacity* to *Current charge* in Battery graph (thanks to @Photon89)
- Standalone, Indicator, Systray: added command line option `-a`, `--about` (open *About* dialog on startup)
- Show copyright notice on application startup (only when target outputs to terminal)

### Bugfixes
- Fixed: failed assert in Memory graph caused abnormal termination in Ubuntu 12.04 (thanks to @spvkgn)
- Fixed: GTK+2 indicator was locked to 120 horizontal pixels (thanks to @leoheck)

### Notes about upgrade
This is a minor release. When upgrading from previous major releases, it is really recommended to read [release notes of Multiload-ng 1.5.0](#multiload-ng-v150).










## Multiload-ng v1.5.0

<sup>*Released on 15 Nov 2016*</sup>

### Changelog
- Reorderable graphs
- New graph: **Battery**
- Remove *timespan* column in Preferences window (thanks to @doublemeat)
- Remove of preferences item *Fill space between graphs*
- Removed icons from menu items to comply with GNOME HIG (Human Interface Guidelines)
- Support for older versions of LxPanel (starting from v0.5.8)
- New color scheme: **Super Mario Bros** (with colors from 1985 game)
- New color scheme: **Dragon Ball Z**
- New color scheme: **The Simpsons**
- Add byte count to detailed tooltip in Swap graph (thanks to @spvkgn)
- Show size of block devices in Disk graph filter selection
- Higher level of optimization (build with `-O3`)
- Official packages for Arch Linux are in AUR
- Install autostart entries by default (systray, indicator)
- Ebuilds (Gentoo): change category from `x11-misc` to `x11-plugins`, add `autostart` USE flag
- PKGBUILD generator does no longer require Arch to run
- Other minor improvements

### Bugfixes
- Fixed: byte sizes over 4 GiB were not formatted properly (thanks to @makrochip)
- Fixed: numeric values could not be directly inserted into *interval* and *size* entries (thanks to @doublemeat)
- Fixed: `tooltip-style` property on MATE plugin not restoring after restart (thanks to @spvkgn)
- Fixed: user-defined colors not restoring after restart (thanks to @spvkgn and @doublemeat)
- Fixed: some translations were missing in MATE plugin's right click menu (thanks to @spvkgn)
- Secured whole code through GCC sanitizers
- Improved debugging experience
- Other minor fixes

### Release notes
This release brings some *under-the-hood* improvements, along with the visible changes.

#### Reorderable graphs
Graphs can now be reordered from a new control in Preferences window, either by dragging them or by
clicking buttons on the side. As a side effect, the notion of *graph index* is now meaningless, so
the `%n` token is not recognized anymore in double click command line.

#### Battery graph
There is a new graph in Multiload-ng: **Battery** graph. It draws system battery usage
(as long there is a battery installed, of course). Graph lines change color according to
battery remaining capacity and charging status. Tooltip can show battery brand and model,
along with its charge.

#### New color schemes
In honor of old times, this release comes with a new series of color schemes inspired
by fictional characters from old video games and TV programs:
- *Super Mario Bros*, with colors of characters and game objects from original 1985 game
- *Dragon Ball Z*, with colors of some DBZ heroes and villains
- *The Simpsons*, with colors of some distinguishing characters

This list is expected to grow in future releases, with many color schemes that
will bring back our childhood memories.

#### Removal of some UI elements
*Timespan* column was removed from Preferences window, because it was considered useless
by some users, and its name was potentially misleading. This saves space and unclutters UI.

Preferences item *Fill space between graphs* was removed too in this release. Many plugins did not
respect this preference (like LxPanel, Standalone, Systray and GTK+3 XFCE). Also it was another
thing with little use and a confusing and misleading name.

**GNOME HIG** (Human Interface Guidelines) recommends to drop icons in menus unless they are strictly
necessary, and suggests to use them only for *nouns* and not for *actions*. Since menu items
in Multiload-ng are all actions, icons were all dropped in order to better adhere to guidelines.
This helps giving Multiload-ng a native look-and-feel in most desktop environments.

#### Autostart entries
Support for autostart entries (*.desktop* files placed in `/etc/xdg/autostart`) was already present in
previous versions, through ./configure flag `--enable-autostart`. However, being disabled by default,
this feature went unnoticed by most users.  
New policy is to install these entries by default, for *indicator* and *systray* targets, so
these applications will now be executed at startup. This is often the desired behavior. Autostart
entries installation can still be avoided, by using the new ./configure flag `--disable-autostart`.

#### Support for older LxPanel versions
Multiload-ng 1.5.0, like its ancestor [multiload-nandhp](https://github.com/nandhp/multiload-nandhp),
now supports old LxPanel API (starting from lxpanel 0.5.8). That means that Multiload-ng can now be also
run in older systems, like Lubuntu 12.04.

#### Code sanitizing
Code was secured by building it with gcc builtin [sanitizers](https://github.com/google/sanitizers)
(AddressSanitizer, MemorySanitizer and LeakSanitizer). This revealed some minor flaws that might potentially
cause buffer and heap overflows. Although these flaws have never caused any problems, it's good to
have them fixed before they can.

#### Higher optimization level
Whole software is now built with `-O3` compiler flag. Multiload-ng runs faster and smoother than ever,
while using less memory.

Thousands of tests haven't revealed any issue with the advanced optimization.

#### Arch Linux
Internal package generator for PKGBUILDS located ([here](https://github.com/udda/multiload-ng/blob/master/extras/pacman/generate-pkgbuild-files.sh))
now produces valid and perfectly compliant scripts, thanks to @Photon89 useful improvements.
It no longer makes use of `makepkg`, now it's a pure bash script that can be run from any distribution.

Automatically generated packages are now mature enough to get *official* status. Official packages for Arch Linux
can be now found in AUR, just [search for them](https://aur.archlinux.org/packages/?SeB=n&K=multiload-ng)!












## Multiload-ng v1.4.2

<sup>*Released on 16 Oct 2016*</sup>

### Changelog
- Compatibility with Ubuntu 12.04 and newer (thanks to @spvkgn)

### Notes about upgrade
This is a minor release. When upgrading from previous major releases, it is really recommended to read [release notes of Multiload-ng 1.4.0](#multiload-ng-v140).










## Multiload-ng v1.4.1

<sup>*Released on 14 Oct 2016*</sup>

### Bugfixes
- Fixed: MATE applet won't start due to missing GSettings keys (thanks to @hotice)

### Notes about upgrade
This is a minor release. When upgrading from previous major releases, it is really recommended to read [release notes of Multiload-ng 1.4.0](#multiload-ng-v140).










## Multiload-ng v1.4.0

<sup>*Released on 10 Oct 2016*</sup>

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

<sup>*Released on 21 Sep 2016*</sup>

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

<sup>*Released on 17 Sep 2016*</sup>

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

<sup>*Released on 28 Aug 2016*</sup>

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

<sup>*Released on 20 Aug 2016*</sup>

Changelog:
* Full GTK3 support
* Build system improvements
* Other fixes










## Multiload-ng v1.0.1

<sup>*Released on 14 Aug 2016*</sup>

Changelog:
* Successful build on Debian / *buntu
* Fixed some gcc warnings
* Other fixes










## Multiload-ng v1.0

<sup>*Released on 3 Aug 2016*</sup>

First stable release of Multiload-ng.

Warning: this fails to build on Debian/Ubuntu.