# Multiload-ng

## Overview
Multiload-ng is a near-complete rewrite of the good old GNOME multiload applet.
<!--TODO plugin screenshots, into all panels - change shape and colors too, to illustrate every possibility-->
<!--TODO screenshot of preferences window too-->
It has plugins for the following panels:
- XFCE 4 (xfce4-panel)
- LXDE (lxpanel)
- MATE (mate-panel)

In addition it can be built as a standalone window, that is, not embedded in any panel.



## Features
- Draw graphs of system resources
- Very customizable
- Automatically adapts to container changes (panel or window)
- Written in pure C with few dependencies - little CPU/memory footprint
- Custom actions on double click



## The Graphs
#### CPU Graph
Draws CPU usage, differentiating between User/System/Nice/IOWait.
#### Memory Graph
Draws RAM usage, differentiating between memory used by the applications
(directly and through shared modules) and memory used as cache/buffers.
#### Network Graph
Draws network I/O of every supported network interface, differentiating
between input, output and local (loopback, adb, etc) traffic.
#### Swap Graph
Draws swap usage, when detected.
#### Load Average Graph
Draws load average, as returned by `uptime`.
#### Disk Graph
Draws Disk I/O, differentiating between read and write.
#### Temperature Graph
Draws temperature of the system, based on the hottest temperature
detected among the supported sensors in the system.




## System Requirements
#### Common requirements
These are the packages required to build any version of the plugin.
See per-panel section below for full list.

Package                     | Min version
:-------------------------- | -------------:
GTK+                        | >= 2.18.0
Cairo                       | (shipped with GTK)
LibGTop                     | >= 2.11.92

#### Requirements for XFCE4 panel
In addition to common requirements (see above)
these packages are required to build XFCE4 panel plugin:

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
