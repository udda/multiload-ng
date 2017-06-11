# Multiload-ng 2

This is the `2.x` branch of Multiload-ng.
(a better README will follow)

That's the next major release of popular graphical system monitor.

### Disclaimer

Multiload-ng 2.x is still at early development stage.
At the moment it's meant for beta testers and enthusiasts only.

I repeat: **IT'S A BETA PRODUCT**. Please keep that in mind when you
experience any problems. Don't use yet Multiload-ng 2.x in production
environments.

## Modular structure

In order to be more flexible and easily portable to most desktop environments,
Multiload-ng 2.x is split in several parts:

- **Core library** (`libmultiload`)
- **Auxiliary libraries** (for example `libmultiload-gtk`, coming soon)
- **Targets** (end user applications, that use libraries above)

Core library contain Multiload-ng logic (data collection and graph drawing).

Auxiliary libraries are built upon core library, and contain some environment-specific code (e.g. `libmultiload-gtk` contain a custom GtkWidget).

Targets are the actual applications (panel plugins, server, ...). They are built
either upon one auxiliary library, or directly upon core library.

### Proposed design

This is the proposed final design. Some elements do not exist at all (but
thanks to the new modular structure, they could).

![This image can be imported in draw.io](misc/resources/structure-diagram.png)

## How to install

First, you have to install `libmultiload`. It's needed by all other software.
Refer to README located in libmultiload directory.

Then you'll have to install your required target. If the target requires an
auxiliary library, it must be installed first. Again. refer to README files in each directory.

## Wanna help?

Multiload-ng 2.x is beta. You can help development by reporting bugs.

Your bug reports can be way more useful if they come with some debug info
from your system.  
With Multiload-ng 2.x you can easily get those info in a single ZIP file.

### Collecting debug info

`libmultiload` ships with an useful software called **multiload-debug-collector**.
Use it to generate a ZIP file with useful information about your system and
Multiload executables.

After installing libmultiload, just run:

	multiload-debug-collector issue.zip

(`issue.zip` can be any valid filename)

**NO PERSONAL INFORMATION AT ALL IS COLLECTED**. If you want, you can inspect
contents of issue.zip before sending it to me.

## Projects that use Multiload-ng

Modular design of Multiload-ng allows everyone to use `libmultiload` for its own project.
libmultiload itself ships with development headers and detailed documentation.

### Wanna use libmultiload?

If you use libmultiload in your project, please let me know, so I can mention
you and the project in this README. You can [send an email](mr.udda@gmail.com) or [create an issue](https://github.com/udda/multiload-ng/issues/new) possibly providing a link to the project homepage / source code.

## Links

- [Multiload-ng community on Google+](https://plus.google.com/u/0/communities/106518305533935900936)

## Acknowledgements
Multiload-ng is inspired by old GNOME Multiload applet, current branch (1.x) is a fork of [nandhp's port to XFCE and LxPanel](https://github.com/nandhp/multiload-nandhp).  
However, starting from version 2.0, Multiload-ng is completely new code, written from scratch. Not a single line has been copied from other projects.

### Third party software

Multiload-ng makes use of third party software libraries, directly embedded
in source code in order to avoid external dependencies when possible.

The full list of libraries (and licenses) is available [here](libmultiload/third-party/README.md).

## Donate

Multiload-ng is a single-person work. I write some code almost daily.

If you like Multiload-ng, and you want support development, or just buy
me a coffee or a beer, there are multiple options available.

Your donations will be really much appreciated. They will keep me motivated to
continue development of Multiload-ng.

### PayPal

Donate to Multiload-ng project with PayPal.
Credit card donations are also accepted.

Donate with PayPal [here](https://paypal.me/udda).

### Bitcoin

Donate to Multiload-ng project with Bitcoin.

Bitcoin (BTC) is an online digital currency that is based on an open-source,
peer-to-peer encryption protocol.  
Bitcoin is not managed by any government or central authority. Instead, it
relies on an Internet-based network.  
For more details, see Wikipedia’s [Bitcoin](http://en.wikipedia.org/wiki/Bitcoin) article.

Bitcoin address: [17BU3bPadWhmMwVe1heCs77wAY5SCMZJw8](bitcoin:17BU3bPadWhmMwVe1heCs77wAY5SCMZJw8?label=Multiload-ng%20Donation)

Alternatively, you can scan this QR code with any Bitcoin app:

![Bitcoin Multiload-ng QR code](misc/resources/bitcoin-qr.png)
