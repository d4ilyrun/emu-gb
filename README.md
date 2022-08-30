# emu-gb

A Game Boy emulator written from scratch in C.

## Building

### Requirements

- gcc
- make __OR__ cmake

If using Nix, you can download all requirements through `nix develop`.

### Linux / MacOs

You can build the project using either Make or CMake.

```sh
# Using make
make

# Using CMake
mkdir build
cd build && cmake ..
make -j4 gb-emu
```

## Usage

```
Usage: emu-gb [OPTION...] CARTRIDGE
GB-EMU: Yet another gameboy emulator written in C

  -l, --log-level=LEVEL      Do not output logs of lower importance
  -s, --silent               Do not show any log
  -t, --trace                Output traces during execution
  -b, --blargg               Display the result of blargg's test roms
  -x, --exit-infinite-loop   Stop execution when encountering an infinite JR
                             loop
  -?, --help                 Give this help list
      --usage                Give a short usage message
```

## TODO

See [TODO](TODO.md)
