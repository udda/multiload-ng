# Multiload-ng

## Overview
Multiload-ng is a modern graphical system monitor. It's a near-complete rewrite of the good old GNOME multiload applet.

It supports the following panels:
- XFCE (xfce4-panel)
- LXDE (lxpanel)
- MATE (mate-panel)
- Standalone (has its own window, not embedded in any panel)

Multiload-ng can be built with GTK2 and GTK3, so can be embedded within GTK2/GTK3 builds of all the panels above.



## Contents
- [Features](#features)
- [Screenshots](#screenshots)
- [The Graphs](#the-graphs)
- [History](#history)
- [System Requirements](#system-requirements)
- [Build Instructions](#build-instructions)
- [How to Contribute](#how-to-contribute)
- [Help & Troubleshooting](#help--troubleshooting)
- [FAQ](#faq)
- [Credits](#credits)



## Features
- Draw graphs of system resources
- Customizable under every aspect
- Builtin color schemes
- Independent configuration for each graph
- Automatically adapts to container changes (panel or window)
- Written in pure C with few dependencies = little CPU/memory footprint
- Customizable tooltips with lots of information
- Customizable mouse actions, with user-defined command lines
- Customizable graph scale
- Customizable data sources (filters)



## Screenshots
![Default configuration, small panel. All graphs visible](../gh-pages/screenshots/0_default.png?raw=true)  
<sup>*Small panel, default configuration. All graphs visible*</sup>

![All graphs visible, larger panel. Thicker (2px) borders](../gh-pages/screenshots/1_borders.png?raw=true)  
<sup>*All graphs visible, larger panel. Thicker (2px) borders*</sup>

![Each graph has different border width. CPU graph has grey-blue gradient background](../gh-pages/screenshots/2_gradient+singleborders.png?raw=true)  
<sup>*Each graph has different border width. CPU graph has grey-blue gradient background*</sup>

![Graphs have no borders. Extra spacing and padding](../gh-pages/screenshots/3_margins+padding+noborder.png?raw=true)  
<sup>*Graphs have no borders. Extra spacing and padding*</sup>

![Graph background matches panel background. Very stylish](../gh-pages/screenshots/4_background-match.png?raw=true)  
<sup>*Graph background matches panel background. Very stylish*</sup>

![Graphs have different sizes. Color scheme: Solarized Light](../gh-pages/screenshots/5_sizes_solarized.png?raw=true)  
<sup>*Graphs have different sizes. Color scheme: __Solarized Light__*</sup>

![Color scheme: Ubuntu Ambiance](../gh-pages/screenshots/6_ambiance.png?raw=true)  
<sup>*Color scheme: __Ubuntu Ambiance__*</sup>

![Standalone window, horizontal layout. Color scheme: Earth](../gh-pages/screenshots/7_standalone_horiz_earth.png?raw=true)  
<sup>*Standalone window, horizontal layout. Color scheme: __Earth__*</sup>

![Standalone window, vertical layout. Color scheme: Solarized Dark](../gh-pages/screenshots/8_standalone_vert_solarized.png?raw=true)  
<sup>*Standalone window, vertical layout. Color scheme: __Solarized Dark__*</sup>



## The Graphs
#### CPU
Draws CPU usage, differentiating between cycles used by programs with normal and
low priority, by the kernel and waiting for I/O to complete.

Tooltip shows CPU information (model, cores, frequency/governor).

#### MEMORY
Draws RAM usage, differentiating between memory used by the applications (directly
and through shared modules) and memory used as cache/buffers.

Tooltip prints these values as percentage and absolute value.

#### NETWORK
Draws network I/O of every supported network interface, differentiating between
input, output and local (loopback, ADB, etc) traffic.

User can choose which interfaces will be used for graph calculations.

#### SWAP
Draws swap usage, when swap is present.

#### LOAD AVERAGE
Draws load average, as returned by `uptime`.

Tooltip shows load average of 1,5,15 minutes and number of active processes/threads.

#### DISK
Draws Disk I/O, differentiating between read and write speeds.

User can choose which partitions will be used for graph calculations.

#### TEMPERATURE
Draws temperature of the system

User can choose which sensor/driver to read for drawing the graph, or let Multiload-ng 
utomatically select the hottest temperature detected among all detected sensors.

#### PARAMETRIC
Draws numeric output of user defined command line. Up to 4 values will be shown together.  
Can be also used to monitor changes to a file using `cat <filename>` as command line.  
Can be also used to execute arbitrary shell commands using `sh -c "<commands>"` as command line.

Tooltip shows contents of command's *stderr*.

#### Stay tuned
Other graphs are coming!



## History
Multiload-ng started as a simple port of multiload-nandhp to lxpanel>0.7.
As I become familiar with code, I started making other little changes, and cleaning the code.
I then contacted original author, but received no reply - meanwhile the plugin continued improving.

This came to the point where the changes became many and deep, and I realized that this wasn't the same project anymore.
I knew that a fresh start would give a boost to development, and at the same time it
would allow to choose future directions with more ease.

For the above reasons, I made Multiload-ng a separate project.
The name changes too (so the filenames), in order to allow them to be installed together.

Multiload-ng gained popularity starting from version 1.1.0, that introduced GTK3 support. Some Linux blogs
started writing about the plugin, and some contributors started to send translation in their languages.

All this keeps the author motivated, and the project alive and kicking!



## System Requirements
#### Common requirements
These are the packages required to build any version of the plugin.
See per-panel section below for full list.

Package                     | Min version
:-------------------------- | -------------:
gtk+                        | >= 2.18.0
cairo                       | >= 1.0

Multiload-ng requires a reasonably recent Linux kernel (>2.6.36) built with
specific configuration options. These are not build-time requirement, rather
run-time ones. Here are required options:  
- ***CONFIG_SYSFS* (sysfs filesystem support)**
- ***CONFIG_PROC_FS* (/proc filesystem support)**

Multiload-ng measures system resources using nodes in /sys and /proc, so they must exist.

In addition, enabling the following options allows Multiload-ng to gather all possible
informations from the system. These are not strictly required, but some graphs might
not work properly, or not work at all, without these other options:  
- ***CONFIG_CPU_FREQ* (CPU Frequency scaling)**
- ***CONFIG_HWMON* (Hardware Monitoring support)**
- ***CONFIG_THERMAL* (Generic Thermal sysfs driver)**
- ***CONFIG_SENSORS_\** ** - enable sensors you need

Any modern kernel (since 2010) sets all these options automatically, so generally
speaking you don't have to worry about user kernels.

#### Requirements for standalone window
Standalone target has no additional requirements.

#### Requirements for LXDE panel
In addition to common requirements (see above)
these packages are required to build LXDE panel plugin:

Package                     | Min version
:-------------------------- | -------------:
lxpanel                     | >= 0.7.0
libfm                       | >= 1.2.0

Due to an error in lxpanel source, if you are using lxpanel 0.7.0 you will
need also libmenu-cache. This was fixed in version 0.7.1. Read about this
[here](http://wiki.lxde.org/en/How_to_write_plugins_for_LXPanel#Preconditions).

You will have to force GTK+2 build (see [here](#gtk-version) for instructions).

Note that LXDE 0.7 or greater is required.

#### Requirements for MATE panel
In addition to common requirements (see above)
these packages are required to build MATE panel plugin:

Package                     | Min version
:-------------------------- | -------------:
libmatepanelapplet-4        | >= 1.7.0

Check which GTK+ version is supported by your panel: mate-panel used to be GTK+2,
newer versions of mate-panel are GTK+3. You may have to set correct GTK+ version
(see [here](#gtk-version) for instructions).

Note that MATE 1.7 or greater is required.

#### Requirements for XFCE panel
In addition to common requirements (see above)
these packages are required to build XFCE panel plugin:

Package                     | Min version
:-------------------------- | -------------:
libxfce4panel               | >= 4.6.0
libxfce4util                | >= 4.6.0
libxfce4ui-1 OR libxfcegui4 | >= 4.8.0

Check which GTK+ version is supported by your panel: xfce4-panel used to be GTK+2
only, newer versions of xfce4-panel supports GTK+3 too. You may have to set
correct GTK+ version (see [here](#gtk-version) for instructions).

Note that XFCE 4.6 or greater is required.




## Build Instructions

#### Get the source
Execute the following command line (you must have git installed):  
`git clone https://github.com/udda/multiload-ng`

If you don't have git, download the lastest source ZIP [here](https://github.com/udda/multiload-ng/archive/master.zip).

Or, if you don't want any surprise, download a stable release [here](https://github.com/udda/multiload-ng/releases).

#### Configure
Move to the directory that contains source code just cloned and run:  
`./autogen.sh`

Now run configure script:  
`./configure --prefix=/usr`  
Change prefix as needed (/usr is the default of most distros and it's just OK; if not specified it defaults to /usr/local).

Lubuntu users (and possibly others) have different libraries location. They may have to set directly *libdir*:  
`./configure --libdir=/usr/lib/x86_64-linux-gnu`  
If plugin does not show up in the list of panel plugins, you could try to repeat build process with this configure setting.

#### GTK version
Build system automatically selects highest GTK+ version available. If you need to build against a lower version, you have to
set it manually (see next section, [Advanced configure](#advanced-configure))

For example, XFCE and LXDE users need GTK2 version of the plugin. They need to add option `--with-gtk=2.0` to *./configure*.

#### Advanced configure
If you are ok with default settings, you can skip this paragraph and head to [Build](#build) section. Otherwise, continue reading.

Configure script automatically detects installed panels (and related development packages) and enables panel plugins accordingly. You can force enable/disable them using `--with-PLUGIN`, `--without-PLUGIN` or `--with-PLUGIN=yes|no` (replace `PLUGIN` accordingly)

Multiload-ng's ./configure has some extra options:  

Option                      | Description                          | Note
--------------------------- | ------------------------------------ | ---------
`--with-gtk=2.0|3.0|auto`   | GTK+ version to compile against      | Remember to set same GTK+ version of the target panel! Otherwise you could get linking or runtime errors.
`--disable-deprecations`    | Disable GDK/GTK deprecation warnings | Useful when build system enforces *-Werror*. Some deprecated symbols does not have adequate replacement yet.
`--enable-experimental`     | Compile in experimental code         | May be unstable/malfunctioning. Only for testing/developing purposes.
`--enable-debug`            | Allows debugging with GDB            | This enables required CFLAGS.
`--enable-profiling`        | Allows profiling with gprof          | This enables required CFLAGS

To get a list of all available options, type:  
`./configure --help`

When you are satisfied with your flags, run `./configure` with selected options.

#### Build
This is simple. Move to the directory that contains source code and execute:  
`make`

#### Install/uninstall
To install (must run `make` before), execute:  
`sudo make install`

To later uninstall you need source directory. If you deleted it, just download again, and run [Configure](#configure) part. Then execute:  
`sudo make uninstall`



## How to Contribute
You can contribute in several ways:

* Suggest new features
* Report bugs
* Translate to new languages
* Port the plugin to other panels
* Submit your personal color scheme
* Implement something present in the [Wishlist](../../wiki/Wishlist)
* Fix something present in [Known issues and TODO](../../wiki/Known-issues-and-TODO)
* ...

Look at the [Wiki](../../wiki), contains all the informations you will need.

Each pull request will be considered and will get a response.




## Help & Troubleshooting
Look at the [FAQ](#faq) for some common pitfalls. All additional documentation is located in the [Wiki](../../wiki).

#### Plugin shows only a vertical blank line
Probably you compiled against the wrong GTK version. E.g: XFCE and LXDE panels are still GTK2-based, and
Multiload-ng build system automatically selects GTK3 if available.

Try running Configure with the right options (see [Configure](#configure) section above)

#### High CPU usage
This has basically two causes:  
* low update interval
* parametric command

Lowering update interval means more redraws per second. CPU usage might become noticeable on older systems.

Command line of parametric graph is called synchronously every time the graph is redrawn. This means that
plugins hangs waiting for the command line to terminate and return numbers. You should try to




## FAQ

#### Q: Which are the differences with original Multiload applet?
A: First of all, this project is *forked* from original Multiload. Nearly 100% of the code has been
rewritten by now, but Multiload-ng is designed to keep ALL the features of the original multiload,
and extend them.

There are some notable differences:

* Original multiload contains old and unmantained code, Multiload-ng is actively mantained
* Multiload-ng has a [Wiki](../../wiki)!
* Multiload-ng runs on a variety of panels, including those of the original multiload
* Multiload-ng recently dropped glibtop dependency, meaning less bugs and less overhead
* Multiload-ng detects automatically which task manager is installed
* Multiload-ng has additional graphs, and more will be added in the future
* Multiload-ng has more graphical customizations, like individual size/interval/border
* Multiload-ng has color schemes support
* Multiload-ng responds to mouse events with per-graph customizable actions
* Multiload-ng can choose its orientation regardless of panel orientation
* Multiload-ng can set graphs scale manually
* Multiload-ng has higher limits for graph size and update interval
* Multiload-ng can also be run without any panel
* Multiload-ng can filter which source to show in suitable graphs
* ...and so on

Try it in your system and you won't come back!

#### Q: Doesn't a system monitor use system resources by itself?
A: Yes. This is true for every system monitor. That's why resources usage from Multiload-ng is kept to a negligible level.

#### Q: I found a bug/I have a suggestion! How can I report?
A: The preferred way to report a bug or suggest new features is by [creating a new issue](../../issues/new).

First, check wether the bug/suggestion is already present in [issues list](../../issues) or in the project [Wishlist](../../wiki/Wishlist).
If it's not, you should [create a new issue](../../issues/new).

#### Q: Will you continue the development of Multiload-ng?
A: Of course! To get an idea of future directions, take a look at the [Wishlist](../../wiki/Wishlist).

#### Q: Why don't you port to panel *\[insertyourpanelhere\]*?
A: Because of a number of reasons:

1. I don't have the time -> Will be done when I find some time
2. I don't have required knowledge -> Will be done when I learn it
3. I didn't know that *\[insertyourpanelhere\]* existed/supported plugins -> Now I know, I'll investigate and eventually you will have your plugin
4. Requires too much work -> If it's worth it, see #1
5. Requires plugins to be written in languages other than C -> Sorry, this would break ALL existing plugins. Unless special cases (like supersets of C or simple wrappers), it's very likely it can't be done.

The best way to get a new port is to suggest it (or code it yourself, of course). Feel free to submit an issue about your request, it will be considered carefully.



## Credits
- FSF and creators of original Multiload applet (see AUTHORS file), for giving me a starting point
- Translators, for sending me always up-to-date translations. Read their names in *About* dialog, in git commits comments, or looking at the source, in the PO file headers
- Beta testers, for reporting bugs that I would have never discovered, because they didn't happen in any of my systems. Now these bugs are all gone, thanks to them. Some translators also helped me with testing.
- [Ethan Schoonover](http://ethanschoonover.com/), for its [Solarized color scheme](http://ethanschoonover.com/solarized)
- [Gisela at All Free Designs](http://allfreedesigns.com/author/123ggizelle/), for [these tips](http://allfreedesigns.com/bright-color-palettes/) I picked to create Fruity color scheme
