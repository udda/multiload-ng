## How to update cJSON

[cJSON](https://github.com/DaveGamble/cJSON/) is a JSON parser used for loading
and storing Multiload-ng configuration.

It consists of two files:

- `cJSON.h`: public definitions
- `cJSON.c`: engine implementation

cJSON is released under MIT license, making it possible to embed its source
into other programs (this is what Multiload-ng does). License is available
for reading in `LICENSE` file.

However, copied source means that updates have to be done manually.

### Update steps

Here is how to update cJSON to lastest version:

1. Download lastest release from cJSON [releases page](https://github.com/DaveGamble/cJSON/releases)
2. You will get a file named like that: `cJSON-1.5.1.tar.xz`
3. Extract `cJSON.c` and `cJSON.h` into this directory
4. Test new cJSON version doing something that produces or parses JSON output
5. Remember to update `version` file with new version number
6. cJSON updated!
