multiload-nandhp
================

multiload-nandhp is a port of the GNOME multiload applet to the Xfce4 and LXDE
panels.

It supports Xfce 4.6 and above, and lxpanel 0.7 and above.

System Requirements
-------------------

- GTK+                          >= 2.18.0
- Cairo
- LibGTop                       >= 2.11.92

For temperature support:
- Linux kernel with CONFIG_THERMAL (Generic Thermal sysfs driver).
  Any modern kernel (since 2010) should be just fine.

For the Xfce panel plugin:

- libxfce4panel                 >= 4.6.0
- libxfce4util                  >= 4.6.0
- libxfce4ui-1 OR libxfcegui4   >= 4.8.0

For the lxpanel plugin:

- lxpanel                       >= 0.7
- libmenu-cache     (Required by lxpanel)

