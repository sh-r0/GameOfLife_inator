# GameOfLife_inator
Program that simulates popular cellular automata - [Conway's Game of life](https://en.wikipedia.org/wiki/Conway's_Game_of_Life), made with openGL and GLFW.

## Usage
Program currently supports 1 parameter - size of map which is passed as 1st argument e.g.:  
./GameOfLife_inator.exe 4000

## Building

### Prerequisites
To build following project it's necessary to install/build glew (https://glew.sourceforge.net) and glfw (https://www.glfw.org/download.html)

### Setting up CMakeLists
In CMakeLists.txt set paths to glew and GLFW library directories in CONFIG INFORMATION section  

## Dependencies
* glew - https://glew.sourceforge.net
* GLFW - https://www.glfw.org