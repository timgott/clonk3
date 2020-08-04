# clonk3

Port of the game Clonk 3 for MS-DOS to modern platforms.
Website of the original: http://clonk.de/classics.php

>Clonk is a multiplayer game of action, tactics and skill. This version can
>be played by up to three players at a time. The game will be played in real
>time. Each player will be able to control his crew of clonks in the randomly
>generated or user defined worlds, fighting his human opponents and/or the
>forces of nature. There are numerous modes and options to provide an almost
>unlimited variety of rounds to be challenged.
>
>Start out by playing the tutorial missions to learn how to play the game.
>Check the on-line help system of Clonk for more information.

(from CLONK3.TXT that was distributed with the game)

## Tips

- Add `-scale [number]` as command line argument to set the scaling factor
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
