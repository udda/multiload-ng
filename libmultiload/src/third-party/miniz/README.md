## How to update Miniz

[Miniz](https://github.com/richgel999/miniz) is the the zlib/deflate engine used
by Multiload-ng for PNG and ZIP data.

It consists of two files:

- `miniz.h`: public definitions
- `miniz.c`: engine implementation

Miniz is released under MIT license, making it possible to embed its source
into other programs (this is what Multiload-ng does). License and authors
are available for reading in the first lines of `miniz.c`.

However, copied source means that updates have to be done manually.

### Update steps

Here is how to update Miniz to lastest version:

1. Download lastest release from Miniz [releases page](https://github.com/richgel999/miniz/releases)
2. You will get a file named like that: `miniz-2.0.5.zip`
3. Extract `miniz.c` and `miniz.h` into this directory
4. Test new Miniz version doing something that produces ZIP or PNG output
5. Remember to update `version` file with new version number
6. Miniz updated!

## Note about compilation errors

Current Miniz version fails to build with GCC flag `-fstrict-aliasing`, raising the error:  
> error: dereferencing type-punned pointer will break strict-aliasing rules [-Werror=strict-aliasing].

Strict aliasing is an optimization option that won't be gone just to make third
party code happy. To successfully build, comment the following line in `miniz.h`:  
`#define MINIZ_USE_UNALIGNED_LOADS_AND_STORES 1`

This way code that violates strict aliasing won't be compiled in.

## Note about LFS (Large File Support)

You may encounter this message during build:  
> #pragma message: Using fopen, ftello, fseeko, stat() etc. path for file I/O - this path may not support large files.

This is related to missing **LFS** (Large File Support), that is a set of functions
which work on files larger than 2 GB (`fopen64`, `fseek64` and so on). To enable
LFS, just define the following macro at the beginning of `miniz.h` (before any include in this file):  
`#define _LARGEFILE64_SOURCE 1`

## Note about version string

`miniz.h` defines the macro `MZ_VERSION` that should be used to retrieve library version.  
Unfortunately, it's set to a different value (`"10.0.0"`). Until upstream developer
fixes that, you'll have to correct this number with the same one that you typed into
`version` file.
