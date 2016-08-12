# Multiload-ng

## Overview
Multiload-ng is a modern graphical system monitor. It's a near-complete rewrite of the good old GNOME multiload applet.

It supports the following panels:
- XFCE (xfce4-panel)
- LXDE (lxpanel)
- MATE (mate-panel)

In addition it can be built as a standalone window, that is, not embedded in any panel.



## Features
- Draw graphs of system resources
- Very customizable
- Color schemes support
- Automatically adapts to container changes (panel or window)
- Written in pure C with few dependencies - little CPU/memory footprint
- Customizable tooltip style
- Custom actions on double click



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

![Standalone window, all graphs visible](../gh-pages/screenshots/5_standalone.png?raw=true)  
<sup>*Standalone window, all graphs visible*</sup>



## Graphs
- **CPU GRAPH**: draws CPU usage, differentiating between User/System/Nice/IOWait.
- **MEMORY GRAPH**: draws RAM usage, differentiating between memory used by the applications (directly and through shared modules) and memory used as cache/buffers.
- **NETWORK GRAPH**: draws network I/O of every supported network interface, differentiating between input, output and local (loopback, adb, etc) traffic.
- **SWAP GRAPH** draws swap usage, when swap is present.
- **LOAD AVERAGE GRAPH**: draws load average, as returned by `uptime`.
- **DISK GRAPH**: draws Disk I/O, differentiating between read and write.
- **TEMPERATURE GRAPH**: draws temperature of the system, based on the hottest temperature detected among the supported sensors in the system.
- ...others coming soon



## Help & Troubleshooting
Read [this](../../wiki/Configuration) for help about plugin configuration.
All additional documentation is located in the [Wiki](../../wiki).



## History
Multiload-ng started as a simple port of multiload-nandhp to lxpanel>0.7.
As I become familiar with code, I started making other little changes, and cleaning the code.
I then contacted original author, but received no reply - meanwhile the plugin continued improving.

This came to the point where the changes became many and deep, and I realized that this wasn't the same project anymore.
I knew that a fresh start would give a boost to development, and at the same time it
would allow to choose future directions with more ease.

For the above reasons, I made Multiload-ng a separate project.
The name changes too (so the filenames), in order to allow them to be installed together.



## System Requirements
#### Common requirements
These are the packages required to build any version of the plugin.
See per-panel section below for full list.

Package                     | Min version
:-------------------------- | -------------:
gtk+                        | >= 2.18.0
cairo                       | >= 1.0
libgtop                     | >= 2.11.92

#### Requirements for XFCE panel
In addition to common requirements (see above)
these packages are required to build XFCE panel plugin:

Package                     | Min version
:-------------------------- | -------------:
libxfce4panel               | >= 4.6.0
libxfce4util                | >= 4.6.0
libxfce4ui-1 OR libxfcegui4 | >= 4.8.0

Note that XFCE 4.6 or greater is required.

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

Note that LXDE 0.7 or greater is required.

#### Requirements for MATE panel
In addition to common requirements (see above)
these packages are required to build MATE panel plugin:

Package                     | Min version
:-------------------------- | -------------:
libmatepanelapplet-4        | >= 1.7.0

Note that MATE 1.7 or greater is required.

#### Requirements for temperature graph
This is not a build-time requirement, rather a run-time one. The plugin search
sysfs nodes corresponding to thermal zones, chooses the hottest one and draws it
in the graph. The only requirement here is a Linux kernel compiled with
`CONFIG_THERMAL (Generic Thermal sysfs driver)`. Any modern kernel (since 2010)
sets it automatically, so it should be just fine.




## Build instructions

#### Get the source
Execute the following command line (you must have git installed):  
`git clone https://github.com/udda/multiload-ng`

If you don't have git, download the source ZIP [here](https://github.com/udda/multiload-ng/archive/master.zip).

#### Configure
Move to the directory that contains source code just cloned and run:  
`./autogen.sh`

Now run configure script:  
`./configure --prefix=/usr`  
Change prefix as needed (/usr is the default of most distros and it's just OK; if not specified it defaults to /usr/local).

Lubuntu users (and possibly others) have different libraries location. They may have to set directly *libdir*:  
`./configure --libdir=/usr/lib/x86_64-linux-gnu`  
If plugin does not show up in the list of panel plugins, you could try to repeat build process with this configure setting.

Configure script automatically detects installed panels (and related development packages) and enables panel plugins accordingly.  
You can force enable/disable for some panels (and standalone). To learn how, along with other available options, type:  
`./configure --help`

Then run `./configure` with selected options.

#### Build
This is simple. Move to the directory that contains source code and execute:  
`make`

#### Install/uninstall
To install (must run `make` before), execute:  
`sudo make install`

To later uninstall you need source directory. If you deleted it, just download again, and run *Configure* part. Then execute:  
`sudo make uninstall`



## How to contribute
You can contribute in several ways:

* Suggest new features
* Translate to new languages
* Port the plugin to other panels
* Report bugs
* Implement something present in the [Wishlist](../../wiki/Wishlist)
* Fix something present in [Known issues and TODO](../../wiki/Known-issues-and-TODO)
* ...

Look at the [Wiki](../../wiki), contains all the informations you will need.

Each pull request will be considered and will get a response.



## FAQ

#### Q: Which are the differences with original Multiload applet?
A: First of all, this project is *forked* from original Multiload. Multiload-ng has ALL the features of the original multiload, since they share the same codebase.

There are some notable differences:

* Original multiload contains old and unmantained code, Multiload-ng is actively mantained
* Multiload-ng has a [Wiki](../../wiki)!
* Multiload-ng has additional graphs, and more will be added in the future
* Multiload-ng has more graphical customizations, like individual colored border
* Multiload-ng has color schemes support
* Multiload-ng responds to mouse events with customizable actions
* Multiload-ng can choose its orientation regardless of panel orientation
* Multiload-ng can also be run without any panel
* ...and so on

Try it in your system and you won't come back!

#### Q: Doesn't a system monitor use system resources by itself?
A: Yes. This is true for every system monitor. That's why resources usage from Multiload-ng is kept to a negligible level.

#### Q: I found a bug! How can I report?
A: First, check wether this bug is already present in [issues list](../../issues) or in the appropriate [wiki page](../../wiki/Known-issues-and-TODO).

If not, you can then [create a new issue](../../issues/new) or add the bug you found in the [Known issues and TODO wiki page](../../wiki/Known-issues-and-TODO).

#### Q: I have a suggestion that could be useful. How can I report?
A: First, check wether your suggestion is already present in [this list](../../wiki/Wishlist).

If not, you can then add your suggestion in the [Wishlist wiki page](../../wiki/Wishlist).

#### Q: Will you continue the development of Multiload-ng?
A: Of course! To get an idea of future directions, take a look at the [Wishlist](../../wiki/Wishlist).

