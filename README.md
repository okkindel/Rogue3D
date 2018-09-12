# Rogue3D

## A raycaster written in C++ with SFML library. In the future probably a roguelike game, if I will have enough patience and time.

## How to run it:

1. ### Clone the repository: 
    `git clone https://github.com/okkindel/Rogue3D.git`
2. ### Install SFML:
* Debian based: `sudo apt-get install libsfml-dev`
* Windows and others: [Click](https://www.sfml-dev.org/tutorials/2.5/start-vc.php)
3. ### Compile it:
    `make debug`
4. ### Run it:
    `./bin/debug`

## Features:
* 3D map generated from array
* Textured walls
* Simple shading based on distance
* Fog on distance
* Walkig (also side walking)
* Lightning (static at the moment)

## How does it look like:
<p align="center"><img title="game screen" src="https://raw.githubusercontent.com/okkindel/Rogue3D/master/data/screen/1.png" width="60%"></p>
<p align="center"><img title="game screen" src="https://raw.githubusercontent.com/okkindel/Rogue3D/master/data/screen/2.png" width="60%"></p>

## Todo:
* 2D Sprites
* Textured floor and celling
* Simple enemies with AI
