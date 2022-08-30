# TESTS

## Unit tests

Unit tests are done through Google's googletest C++ framework.  
They are located inside the `tests` folder.

## Building

### Dependencies
- gcc/g++
- googletest

### Nix Flakes
You can run `nix build .#unit-tests` at the root of the project.
It will build the tests and move all the executables into the `result/tests` folder.
You can then run the `run-tests.sh` script inside of the `scripts` folder to get a quick overview of the failing tests.

### CMake
```
mkdir build && cd build
cmake .. -DBUILD_TESTS=ON
make help # The test taget have their name finish by '_test' (ie. ld_test)

# Example
# Build tests for the LD instruction to 'tests/cpu/instruction/ld'
make -j4 ld_test
```

## ROMs

I've taken these ROM files from [rokytriton's Game Boy emulator](https://github.com/rockytriton/LLD_gbemu/tree/main/roms).  
To use them it is necessary to run with `--blargg`. They should display wether the test passed (or run indefinitely ...).
