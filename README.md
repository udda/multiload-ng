# Multiload-ng

## Overview
Multiload-ng is a modern graphical system monitor.
It's a near-complete rewrite of the good old GNOME multiload applet, that aims to support *every* existing panel.

It supports the following panels:
- XFCE (*xfce4-panel*)
- LXDE (*lxpanel*)
- MATE (*mate-panel*)
- Ubuntu Unity (through *libappindicator*)
- Every panel with support for [Application Indicators](https://unity.ubuntu.com/projects/appindicators/)
- System tray (virtually *any* panel with a systray, in particular those without external plugins support, like [tint2](https://gitlab.com/o9000/tint2/wikis/home))
- Standalone (has its own window, not embedded in any panel)
- Avant Window Navigator (**EXPERIMENTAL**)

Multiload-ng can be built with GTK2 and GTK3, so can be embedded within GTK2/GTK3 builds of all the panels above.



## Contents
- [Features](#features)
- [Screenshots](#screenshots)
- [The Graphs](#the-graphs)
- [Color Schemes](#color-schemes)
- [History](#history)
- [System Requirements](#system-requirements)
- [Install Instructions](#install-instructions)
- [Build Instructions](#build-instructions)
- [How to Contribute](#how-to-contribute)
- [Help & Troubleshooting](#help--troubleshooting)
- [FAQ](#faq)
- [Disclaimer](#disclaimer)
- [Credits](#credits)



## Features
- Draw graphs of system resources ([learn more](#the-graphs))
- Customizable under every aspect
- Builtin color schemes ([learn more](#color-schemes))
- Independent configuration for each graph
- Automatically adapts to container changes (panel or window)
- Written in pure C with few dependencies = little CPU/memory footprint
- Customizable tooltips with lots of information
- Customizable mouse actions, with user-defined command lines
- Customizable graph scale
- Customizable data sources (filters)



## Screenshots
![Default configuration, small panel. All graphs visible](../gh-pages/screenshots/all.panel-24px.png?raw=true)  
<sup>*Small panel, default configuration. All graphs visible*</sup>

![All graphs visible, larger panel. Thicker (2px) borders, graphs have different widths](../gh-pages/screenshots/all.panel-40px.border-2px.widths.png?raw=true)  
<sup>*All graphs visible, larger panel. Thicker (2px) borders, graphs have different widths*</sup>

![Multiload-ng running on Ubuntu](../gh-pages/screenshots/ubuntu.png?raw=true)  
<sup>*Multiload-ng running on Ubuntu*</sup>

![Multiload-ng running on LxPanel](../gh-pages/screenshots/lxpanel.png?raw=true)  
<sup>*Multiload-ng running on LxPanel*</sup>

![Each graph has different border width. Color scheme: Uranus](../gh-pages/screenshots/borders.uranus.png?raw=true)  
<sup>*Each graph has different border width. Color scheme: __Uranus__*</sup>

![Graphs have no borders. Extra spacing and padding. Color scheme: Solarized Light](../gh-pages/screenshots/margin.padding.noborder.solarized-light.png?raw=true)  
<sup>*Graphs have no borders. Extra spacing and padding. Color scheme: __Solarized Light__*</sup>

![Graph background matches panel background. Very stylish](../gh-pages/screenshots/background-blend.png?raw=true)  
<sup>*Graph background matches panel background. Very stylish*</sup>

![Graphs have vertical orientation on horizontal panel. The opposite is also possible](../gh-pages/screenshots/vertical-on-horizontal.png?raw=true)  
<sup>*Graphs have vertical orientation on horizontal panel. The opposite is also possible*</sup>

![Graphs are contiguous, with no border and no spacing, behaving like a single graph. Color scheme: Jupiter](../gh-pages/screenshots/contiguous.noborder.jupiter.png?raw=true)  
<sup>*Graphs are contiguous, with no border and no spacing, behaving like a single graph. Color scheme: __Jupiter__*</sup>

![Just CPU graph. Color scheme: Ubuntu Ambiance](../gh-pages/screenshots/cpu.ambiance.png?raw=true)  
<sup>*Just CPU graph. Color scheme: __Ubuntu Ambiance__*</sup>

![Standalone window, horizontal layout. Color scheme: Fruity](../gh-pages/screenshots/standalone.horizontal.fruity.png?raw=true)  
<sup>*Standalone window, horizontal layout. Color scheme: __Fruity__*</sup>

![Standalone window, vertical layout. Color scheme: Numix Light](../gh-pages/screenshots/standalone.vertical.numix-light.png?raw=true)  
<sup>*Standalone window, vertical layout. Color scheme: __Numix Light__*</sup>



## The Graphs
### CPU
Draws CPU usage, differentiating between cycles used by programs with normal and
low priority, by the kernel and waiting for I/O to complete.

Tooltip shows CPU information (model, cores, frequency/governor).

### MEMORY
Draws RAM usage, differentiating between memory used by the applications (directly
and through shared modules) and memory used as cache/buffers.

Tooltip prints these values as percentage and absolute value.

### NETWORK
Draws network I/O of every supported network interface, differentiating between
input, output and local (loopback, ADB, etc) traffic.

User can choose which interfaces will be used for graph calculations.

### SWAP
Draws swap usage, when swap is present.

### LOAD AVERAGE
Draws load average, as returned by `uptime`.

Tooltip shows load average of 1,5,15 minutes, number of active processes/threads
and informations about currently running kernel.

### DISK
Draws Disk I/O, differentiating between read and write speeds.

User can choose which partitions will be used for graph calculations.

### TEMPERATURE
Draws temperature of the system.

User can choose which sensor/driver to read for drawing the graph, or let Multiload-ng 
automatically select the hottest temperature measured among all detected sensors.

### BATTERY
Draws charge and status of system battery, using different colors for charging,
discharging and critical level.

Tooltip shows additional battery info.

### PARAMETRIC
Draws numeric output of user defined command line. Up to 4 values will be shown together.  
Can be also used to monitor changes to a file using `cat <filename>` as command line.  
Can be also used to execute arbitrary shell commands using `sh -c "<commands>"` as command line.

Tooltip shows contents of command's *stderr*.

### Stay tuned
Other graphs are coming!



## Color Schemes
Multiload-ng has color scheme support, that is, every color of the graphs can be changed.
This include borders and background (two-color gradient).

Colors (except for border and background) have alpha values, so you can play with
transparency too!

There are also some builtin color schemes, which you can set with a single click.
Some examples are present in [screenshots](#screenshots) above. Here is a partial list:
- **Default color scheme** with its distinctive colors
- **Tango** from [Tango Desktop Project](http://tango.freedesktop.org/)
- **Solarized** by [Ethan Schoonover](http://ethanschoonover.com/solarized) in both Light and Dark variants
- Color schemes inspired by famous Linux distribution: **[Ubuntu](http://www.ubuntu.com/)** (both Ambiance and Radiance), **[Linux Mint](https://www.linuxmint.com/)**
- Color schemes inspired by famous GTK themes: **[Numix](https://numixproject.org/)** (both Light and Dark), **[Arc](https://github.com/horst3180/arc-theme)**
- Color schemes inspired by outer space: **Moon**, **Venus**, **Earth**, **Mars**, **Jupiter**, **Uranus**, **Neptune**
- Color schemes inspired by **The Simpsons** (from the work of designer [Hyo Taek Kim](http://www.hyotk.com/), watch it [here](http://www.fubiz.net/en/2015/09/19/the-colors-of-simpsons-pantone/))
- Color schemes inspired by old cartoons and video games: **Dragon Ball Z**, **Super Mario Bros** (with colors from original 1985 game)
- More!



## History
Multiload-ng started as a simple port of multiload-nandhp to lxpanel>0.7.
As I become familiar with code, I started making other little changes, and cleaning the code.
I then contacted original author, but received no reply - meanwhile the plugin continued improving.

This came to the point where the changes became many and deep, and I realized that this wasn't the same project anymore.
I knew that a fresh start would give a boost to development, and at the same time it
would allow to choose future directions with more ease.

For the above reasons, I made Multiload-ng a separate project.
The name changes too (so the filenames), in order to allow them to be installed together.

Multiload-ng gained popularity starting from version 1.1.0, that introduced GTK3 support. Some Linux bloggers
started writing about the plugin, and some contributors started to send translation in their languages.

All this keeps the author motivated, and the project alive and kicking!



## System Requirements
### Common requirements
These are the packages required to build any version of the plugin.
Some panels may require additional packages (see below).

Package  | Min version (GTK+2)  | Min version (GTK+3)
:------  | ------------------:  | ------------------:
gtk+     | >= 2.20.0            | >= 3.0.0
cairo    | >= 1.0               | >= 1.0

Multiload-ng requires a reasonably recent Linux kernel (>2.6.36) built with
specific configuration options. These are not build-time requirement, rather
run-time ones. Here are required options:  
- **CONFIG_SYSFS** *(sysfs filesystem support)*
- **CONFIG_PROC_FS** *(/proc filesystem support)*

In addition, enabling the following options allows Multiload-ng to gather all possible
informations from the system. These are not strictly required, but some graphs might
not work properly, or not work at all, without these other options:  
- **CONFIG_CPU_FREQ** *(CPU Frequency scaling)*
- **CONFIG_POWER_SUPPLY** *(Power supply class support)*
- **CONFIG_HWMON** *(Hardware Monitoring support)*
- **CONFIG_THERMAL** *(Generic Thermal sysfs driver)*
- __CONFIG_SENSORS_*__ - enable sensors you need

Any modern kernel (since 2010) sets all these options automatically, so generally
speaking you don't have to worry about user kernels.

It is recommended building with **gcc**. Multiload-ng code is pretty standards
compliant, so it should work flawlessly with any C compiler, but using
compilers other than *gcc* is untested and it's not guaranteed to succeed.

### Requirements for Application Indicator (Ubuntu Unity and others)
In addition to common requirements (see above)
these packages are required to build Application Indicator:

Package          | Min version
:------          | ----------:
libappindicator  | >= 0.4.92

Check which GTK+ version is supported by your target panel.
Ubuntu Unity needs AppIndicators to be built against GTK+3.
You may have to set correct GTK+ version (see [here](#gtk-version) for instructions).

### Requirements for LXDE panel
In addition to common requirements (see above)
these packages are required to build LXDE panel plugin:

Package        | Min version  | Notes
:------        | ---:         | :----
lxpanel        | >= 0.5.8     |
libmenu-cache  |              | Only for lxpanel <= 0.7.0
libfm          | 1.2.0        | Only for lxpanel >= 0.7.0


Note that due to a mistake in lxpanel source, if you are using lxpanel 0.7.0
you will need both `libfm` and `libmenu-cache`. This was fixed in version 0.7.1.
Read about this [here](http://wiki.lxde.org/en/How_to_write_plugins_for_LXPanel#Preconditions).

You might have to force GTK+2 build (see [here](#gtk-version) for instructions).

Note that LXDE 0.5.8 or greater is required.

### Requirements for MATE panel
In addition to common requirements (see above)
these packages are required to build MATE panel plugin:

Package               | Min version
:------               | ----------:
libmatepanelapplet-4  | >= 1.7.0

Check which GTK+ version is supported by your panel: mate-panel used to be GTK+2
only, while newer versions of mate-panel are GTK+3. You may have to set correct
GTK+ version (see [here](#gtk-version) for instructions).

Note that MATE 1.7 or greater is required.

### Requirements for XFCE panel
In addition to common requirements (see above)
these packages are required to build XFCE panel plugin:

Package        | Min version (GTK+2)  | Min version (GTK+3)
:------        | ------------------:  | ------------------:
libxfce4panel  | >= 4.6.0             | >= 4.12.0
libxfce4util   | >= 4.6.0             | >= 4.12.0

Check which GTK+ version is supported by your panel: xfce4-panel used to be GTK+2
only, while newer versions of xfce4-panel supports GTK+3 too. You may have to set
correct GTK+ version (see [here](#gtk-version) for instructions).

Note that XFCE 4.6 or greater is required for GTK+2 plugin.
Note that XFCE 4.12 or greater is required for GTK+3 plugin.


## Install instructions

### Package generators
Take a look at [extras directory](https://github.com/udda/multiload-ng/tree/master/extras).
There are scripts for most Linux distributions, that generate packages which can
be then installed through package managers.

### External support
- For Ubuntu/Mint and derivatives, [WebUpd8 PPA](https://launchpad.net/~nilarimogard/+archive/ubuntu/webupd8) hosts **stable** packages for all supported panels.
Read how to install [here](http://www.webupd8.org/2016/10/multiload-ng-140-gnome-twitch-030.html) (instructions are valid for all versions).
- Arch users can get Multiload-ng from AUR. There is a separate package for each variant of the plugin.
Just [search for multiload-ng in AUR](https://aur.archlinux.org/packages/?SeB=n&K=multiload-ng)!

If your distro isn't covered above, or you just want to have some control over build (e.g. enable/disable some plugins, try experimental features), you can [build it yourself](#build-instructions).



## Build Instructions

### Get the source
Execute the following command line (you must have git installed):  
`git clone https://github.com/udda/multiload-ng`

If you don't have git, download the lastest source ZIP [here](https://github.com/udda/multiload-ng/archive/master.zip).

Or, if you don't want any surprise, download a stable release [here](https://github.com/udda/multiload-ng/releases).

### Configure
Move to the directory that contains source code just cloned and run:  
`./autogen.sh`

Now run configure script:  
`./configure --prefix=/usr`  
Change prefix as needed. **/usr** is the prefix of most distros.
If you do not specify a prefix, configure script tries to infer it from installed programs. If this fails, prefix defaults to **/usr/local**.

### LxPanel in systems with Multiarch support
Some Linux distributions (like Debian and its derivatives) have [Multiarch](https://wiki.debian.org/Multiarch) support, means that
they can install 32 bit and 64 bit libraries alongside each other.

This affects libraries location for LxPanel plugin, and it must be set manually.

Users of Multiarch-enabled systems running LxPanel (e.g. Lubuntu) have to set `libdir` directly, by adding to *./configure* the
option `--libdir=/usr/lib/x86_64-linux-gnu` in 64 bit systems, and `--libdir=/usr/lib/i386-linux-gnu` in 32 bit systems.

If plugin does not show up in the list of LxPanel plugins, you could try to repeat build process with this configure setting.

### GTK version
Build system automatically selects highest GTK+ version available. If you need to build against a lower version, you have to
set it manually (see next section, [Advanced configure](#advanced-configure))

For example, LxPanel and older versions of MATE and XFCE panels need GTK2 version of the plugin.
They need to add option `--with-gtk=2.0` to *./configure*.

### Advanced configure
If you are ok with default settings, you can skip this paragraph and head to [Build](#build) section. Otherwise, continue reading.

Configure script automatically detects installed panels (and related development packages) and enables panel plugins accordingly. You can force enable/disable them using `--with-PLUGIN`, `--without-PLUGIN` or `--with-PLUGIN=yes|no` (replace `PLUGIN` accordingly)

Multiload-ng's ./configure has some extra options:  

Option                      | Description                           | Note
--------------------------- | ------------------------------------  | ---------
`--with-gtk=2.0|3.0|auto`   | GTK+ version to compile against       | Remember to set same GTK+ version of the target panel! Otherwise you could get linking or runtime errors.
`--disable-deprecations`    | Disable GDK/GTK deprecation warnings  | Useful when build system enforces *-Werror*. Some deprecated symbols does not have adequate replacement yet.
`--enable-experimental`     | Compile in experimental code          | May be unstable/malfunctioning. Only for testing/developing purposes.
`--enable-debug`            | Allows debugging with GDB             | This enables required CFLAGS.
`--enable-profiling`        | Allows profiling with gprof           | This enables required CFLAGS
`--disable-autostart`       | Do not install autostart entries      | Autostart entries are normally put into `/etc/xdg/autostart` for some plugins (indicator, systray...)

To get a list of all available options, type:  
`./configure --help`

When you are satisfied with your flags, run `./configure` with selected options.

### Build
This is easy. Move to the directory that contains source code and execute:  
`make`

### Install/uninstall
To install (must run `make` before), execute:  
`sudo make install`

To later uninstall you need source directory. If you deleted it, just download again, and run [Configure](#configure) part. Then execute:  
`sudo make uninstall`



## How to Contribute
Read [here](https://github.com/udda/multiload-ng/blob/master/CONTRIBUTING.md)



## Help & Troubleshooting

### Plugin shows only a vertical blank line
Probably you compiled against the wrong GTK+ version. E.g: LXDE panel and older version of MATE and XFCE panels
are still based on GTK+2, and Multiload-ng build system automatically selects GTK+3 when available.

Try running Configure with the right options (see [Configure](#configure) section above)

### High CPU usage
This has basically two causes:  
* low update interval
* parametric command

Lowering update interval means more redraws per second. CPU usage might become noticeable on older systems.

Command line of parametric graph is called **synchronously** every time the graph is redrawn.
This means that plugins hangs waiting for the command to terminate and return numbers.

You should try to raise update interval, and try to use lightweight programs for parametric command line.

### Memory usage is not the same reported by task manager
There is some disagreement on how to count some components of kernel memory. Some resource monitors
(like `htop` and some graphical task managers) count them as *used* memory, while other programs
like `top` and `free` report them as *cached*.

Multiload-ng now allows to choose *where* to count that memory component, in order
to align memory usage value with what your favorite resource monitor reports. You can
find the switch in *Advanced Preferences* window of *Memory* graph.

### Tooltips not showing reliably
This happens often on GTK+3 builds. Due to changes in the way tooltips are handled in GTK+3,
sometimes they might show with big delay, or not show at all. Sometimes they do show up,
but without being updated with every graph redraw, as they should.

Basically, being on GTK+ side, there is not much that can be done from Multiload-ng.
Some users "*solved*" it by raising update interval of the graphs above
*GTK+ default tooltip delay time* (500 ms in many systems).

Starting from GTK+ 3.20.0, tooltips appear to be working normally.

### Plugin doesn't show, and terminal is full of lines like this: `GLib-GObject-WARNING *: cannot register existing type 'GtkWidget'`
You probably selected wrong GTK+ version when building. See [GTK Version section](#gtk-version)




## FAQ

### Q: Which are the differences with original Multiload applet?
A: First of all, this project is *forked* from original Multiload. Nearly 100% of the code has been
rewritten by now, but Multiload-ng is designed to keep ALL the features of the original multiload,
and extend them.

There are some notable differences:

* Original multiload contains old and unmantained code, Multiload-ng is actively mantained
* Multiload-ng has a [Wiki](../../wiki)!
* Multiload-ng runs on a variety of panels, including those of the original multiload
* Multiload-ng does not depend on *glibtop*, meaning less bugs and less overhead
* Multiload-ng detects automatically installed task manager and launches it without configuration
* Multiload-ng has additional graphs, and more will be added in the future
* Multiload-ng has way more graphical customizations
* Multiload-ng has color schemes support
* Multiload-ng responds to mouse events with per-graph customizable actions
* Multiload-ng can choose its orientation regardless of panel orientation
* Multiload-ng can set graphs scale manually
* Multiload-ng has higher limits for graph size and update interval
* Multiload-ng can also be run without any panel
* Multiload-ng can filter which source to show in suitable graphs
* ...and so on

Not enough? Try it in your system and you won't come back!

### Q: Doesn't a system monitor use system resources by itself?
A: Yes. This is true for every system monitor. That's why resources usage from Multiload-ng is kept to a negligible level.

### Q: I found a bug/I have a suggestion! How can I report?
A: It's easy. Read [here](https://github.com/udda/multiload-ng/blob/master/CONTRIBUTING.md)

### Q: Will you continue the development of Multiload-ng?
A: Of course! To get an idea of future directions, take a look at the [Wishlist](../../wiki/Wishlist).

### Q: Why don't you port to panel *\[insertyourpanelhere\]*?
A: Because of a number of reasons:

1. I don't have the time -> Will be done when I find some time
2. I don't have required knowledge -> Will be done when I learn it
3. I didn't know that *\[insertyourpanelhere\]* existed/supported plugins -> Now I know, I'll investigate and eventually you will have your plugin
4. Requires too much work -> If it's worth it, see #1
5. Requires plugins to be written in languages other than C -> Sorry, this would break ALL existing plugins. Unless special cases (like supersets of C or simple wrappers), it's very likely it can't be done.

The best way to get a new port is to suggest it (or code it yourself, of course). Feel free to submit an issue about your request, it will be considered carefully.



## Disclaimer
All product names, logos and brands are property of their respective owners. All company, product and service names used in this software are for identification purposes only. Use of these names, logos, and brands does not imply endorsement, and do not infringe any patent, trademark, copyright, license or any other proprietary right of any third party.



## Credits
- FSF and creators of original Multiload applet (see AUTHORS file), for giving a good starting point
- Translators, for sending me always up-to-date translations. Read their names in *About* dialog, in git commits comments, or looking at the source, in the PO file headers
- Beta testers, for reporting bugs that I would have never discovered, because they didn't happen in any of my systems. Now these bugs are all gone, thanks to them. Some translators also helped me with testing.
