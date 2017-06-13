# libmultiload: Multiload-ng core library

This is a temporary README. You can get a better idea of how Multiload-ng
works in top dicectory [README](../README.md).

## Build requirements

In order to build Multiload-ng, you will need:

- Autotools (`aclocal`, `autoconf`, `autoheader`, `automake`)
- Gettext tools (namely `autopoint`, it's often located in `gettext-tools` package of your distro)
- Libtool
- GCC compiler. Library might compile with other compilers too, but GCC
  is the only one officially supported
- Optional: [Doxygen](http://www.stack.nl/~dimitri/doxygen/) to generate documentation

Consult your distro documentation for detailed instructions for getting
all the requirements installed.

## Dependencies, runtime requirements

`libmultiload` has no external runtime dependencies. You don't have to
install anything on target system.

The only runtime requirement is a Linux kernel, with version starting from
**2.6.36** (included).  
Any system after 2010 is just fine.

## Package contents

libmultiload includes:

- library itself (`libmultiload`), used by all Multiload-ng targets
- development header (`multiload.h`) and pkg-config support, for developing applications using libmultiload
- library documentation (covering general instructions, resources usage,
  internals analysis, and API), requires Doxygen
- a small console utility (`multiload-debug-collector`) that you can use
  to collect useful debug info for bug reports (or you can use it for yourself)
- Multiload-ng icons, in several sizes

## Installation instructions

Once you have all required software (see [above](#requirements)), just follow
the following instructions:

1. Move into this directory with a terminal emulator.
2. Source tree needs to be prepared. Run this command: `./autogen.sh`.  
   If any error, you likely miss one or more [requirements](#requirements).
3. It's time to run the configure script. This script takes a number of
   parameters, you should change them only if you know what you're doing.  
   Below are command lines that work in most cases:
   - Normal users, run this command: `./configure --prefix=/usr`
   - Beta testers, run this command: `./configure --prefix=/usr --enable-debug`

   If succeeds, the configure script will print a large "Multiload-ng" ASCII art,
   followed by build configuration.  
   If any error, please report back to me.
4. You are now ready to build. Just type `make`.  
   If any error, please report back to me.
5. If make succeeded, you just have to install. Run `sudo make install`, and
   type in your user password, if asked.

libmultiload is installed, ready to be used by targets, or by your brand
new application!
