# GameOfLife_inator
Program that simulates popular cellular automata - [Conway's Game of life](https://en.wikipedia.org/wiki/Conway's_Game_of_Life), made with openGL and GLFW.

## Usage
### Flags
Program currently supports different flags:  
* -h/help : flag that outputs possible flags
* -s/-size : sets the size of a map
* -d/density : sets the density of a starting state
* -seed : sets the seed used to generate map

## Building

### Prerequisites
To build following project it's necessary to install/build glew (https://glew.sourceforge.net) and glfw (https://www.glfw.org/download.html)

### Setting up CMakeLists
In CMakeLists.txt set paths to glew and GLFW library directories in CONFIG INFORMATION section  

## Dependencies
* glew - https://glew.sourceforge.net
* GLFW - https://www.glfw.org