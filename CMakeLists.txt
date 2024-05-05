cmake_minimum_required(VERSION 3.20.0)
set (CMAKE_CXX_STANDARD 20)

project(GameOfLife_inator)
add_compile_definitions(CRT_SECURE_NO_WARNINGS)

# CONFIG INFORMATION
set(glewIncDir	"PASTE PATH HERE")
set(glewLibDir	"PASTE PATH HERE")	
set(glfwIncDir	"PASTE PATH HERE")
set(glfwLibDir	"PASTE PATH HERE")
#--CONFIG INFORMATION

list(APPEND LibsList "${glewLibDir}/glew32s.lib" "${glfwLibDir}/glfw3.lib" "opengl32.lib")

set(CMAKE_SOURCE_DIR "${MY_ROOT}/src")
include_directories("${MY_ROOT}/src")
include_directories("${glewIncDir}")
include_directories("${glfwIncDir}")
include_directories("${glfwIncDir}/GLFW")
list(APPEND sourcesList "src/main.cpp" "${glfwIncDir}/GLFW/glfw3.h" "${glewIncDir}/GL/glew.h")
add_executable(${PROJECT_NAME} ${sourcesList})
list(APPEND libsDirList ${glewLibDir} ${glfwLibDir})
link_directories(${PROJECT_NAME} ${libsDirList})
target_link_libraries(${PROJECT_NAME} ${LibsList})