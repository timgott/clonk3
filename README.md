# clonk3

Port of the game Clonk 3 for MS-DOS to modern platforms.
Website of the original: http://clonk.de/classics.php

![Demo](../assets/demo.gif)

## Command line options

- `--scale [number]` sets the scaling factor
- `--interpolation [none|scale2x|scale4x]` sets the interpolation filter type (default is scale2x)
## Tips

- Press F5/F6 to change game speed
- To enable sound you have to select the `SDL Method`

## Dependencies
SDL2

## Compiling
Compile using CMake:

```
mkdir build
cd build
cmake ..
```
