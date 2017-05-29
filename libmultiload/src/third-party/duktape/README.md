## How to update Duktape

[Duktape](http://duktape.org/) is the JavaScript engine used by JavaScript
provider.

It consists of three files:

- `duktape.h`: public definitions
- `duktape.c`: engine implementation
- `duk_config.h`: private engine configuration

Duktape is released under MIT license, making it possible to embed its source
into other programs (this is what Multiload-ng does). License, authors and
version are available for reading in the first lines of `duktape.h`.

However, copied source means that updates have to be done manually.

### Update steps

Here is how to update Duktape to lastest version:

1. Download lastest release from Duktape [website](http://duktape.org/download.html)
2. You will get a file named like that: `duktape-2.1.0.tar.xz`. Extract it into a temporary location, say `/tmp/duktape-2.1.0/`
3. Create another temporary directory where to store configured Duktape sources, say `/tmp/duktape-conf`
4. Run the following commands (you may need to install some Python dependencies):  
   <pre>cd /tmp/duktape-2.1.0/tools
   chmod +x configure.py
   ./configure.py --output-directory=/tmp/duktape-conf --platform=linux --compiler=gcc --omit-removed-config-options --omit-deprecated-config-options --omit-unused-config-options</pre>
5. If command succeeded, directory `/tmp/duktape-conf` now contains some files. Copy back `duktape.h`, `duktape.c` and `duk_config.h` into this directory
6. Test new Duktape version with JS provider
7. Remember to update `version` file with new version number
8. Duktape updated!
