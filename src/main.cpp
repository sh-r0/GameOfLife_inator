#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <array>
#include <vector>
#include <cassert>
#include <unordered_map>

constexpr uint32_t windowSizeX_c = 1024, windowSizeY_c = 1024;
constexpr const char* appName_c = "GameOfLife_inator";
constexpr const char* helpMessage =
"./GameOfLife_inator.exe <flags>\n"
"flags:\n"
"-h/-help - help flag"
"-s/-size <value> - size of map side(map is value^2 sized)\n"
"-d/-density <value> - density of starting map, interval(0-100)\n"
"-seed <value> - seed of generated starting map\n";

float zoom = 1.0f;
float offsetX = 0, offsetY = 0;
size_t mapSize;				//size of map's side -> maps size is mapSize^2 pixels
uint16_t density, seed;
int32_t isPressed = 0;	//which button was pressed last
bool isPaused = false;	//is simulation going

inline void readFile(const std::string& _filepath, std::string& _buff) {
	std::ifstream inFile(_filepath);
	std::stringstream stringStream;
	stringStream << inFile.rdbuf();
	_buff = stringStream.str();
	return;
}

inline uint32_t compileShader(uint32_t _type, const std::string& _src) {
	uint32_t id = glCreateShader(_type);
	const char* source = _src.c_str();
	glShaderSource(id, 1, &source, nullptr);
	glCompileShader(id);
	return id;
}

uint32_t createShaderProgram(const std::vector<uint32_t>& _types, const std::vector<std::string>& _paths) {
	assert(_types.size() == _paths.size() && "vectors sizes differ!");
	uint32_t id;
	id = glCreateProgram();
	std::vector<uint32_t> shaderIds;
	for (size_t i = 0; i < _types.size(); ++i) {
		std::string source; 
		readFile(_paths[i], source);
		shaderIds.push_back(compileShader(_types[i], source));
		glAttachShader(id, shaderIds.back());
	}
	glLinkProgram(id);
	glValidateProgram(id);
	for (uint32_t _id : shaderIds) 
		glDeleteShader(_id);
	return id;
}

struct vertex_t {
	struct {
		float x, y;
	} position;
	struct { 
		float u, v;
	} texCoords;
};

inline void clampOffset(void) {
	if (offsetX < 0)
		offsetX = 0;
	if (offsetY < 0)
		offsetY = 0;

	float dx, dy;
	dx = offsetX + mapSize / zoom - mapSize;
	if (dx > 0) 
		offsetX -= dx;
	dy = offsetY + mapSize / zoom - mapSize;
	if (dy > 0) 
		offsetY -= dy;
	
	return;
}

uint32_t currStateTexId = UINT32_MAX;
uint32_t prevStateTexId = UINT32_MAX;

bool isMousePressed = false;
void mousePressCallback(GLFWwindow* _window, int32_t _button, int32_t _action, int32_t _mods) {
	if (_button != GLFW_MOUSE_BUTTON_LEFT)
		return;
	if (currStateTexId == UINT32_MAX) return;

	double x, y;
	glfwGetCursorPos(_window, &x, &y);
	y = windowSizeY_c - y;

	x = offsetX + x / windowSizeX_c * mapSize / zoom;
	y = offsetY + y / windowSizeY_c * mapSize / zoom;

	if (_action == GLFW_PRESS) {
		float a = 0;
		glTextureSubImage2D(currStateTexId, 0, int32_t(x), int32_t(y), 1, 1, GL_RGBA, GL_FLOAT, &a);
		glTextureSubImage2D(prevStateTexId, 0, int32_t(x), int32_t(y), 1, 1, GL_RGBA, GL_FLOAT, &a);

		isMousePressed = true;
	}
	else isMousePressed = false;


	return;
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	if (yoffset < 0.0f) {
		zoom = zoom == 1.0f ? 1.0f : zoom / 2.0f;
		clampOffset();
	}
	if (yoffset > 0.0f) {
		zoom = zoom * 2.0f;
	}
	return;
}

void processInput(GLFWwindow* _window) {
	if (glfwGetKey(_window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		if (isPressed == 0) {
			offsetX -= mapSize / (2*zoom);
			isPressed = 1;
		}
	}
	else if (glfwGetKey(_window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		if (isPressed == 0) {
			isPressed = 2;
			offsetX += mapSize / (2 * zoom);
		}
	}
	else if (glfwGetKey(_window, GLFW_KEY_UP) == GLFW_PRESS) {
		if (isPressed == 0) {
			isPressed = 3;
			offsetY += mapSize / (2 * zoom);
		}
	}
	else if (glfwGetKey(_window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		if (isPressed == 0) {
			isPressed = 4;
			offsetY -= mapSize / (2 * zoom);
		}
	}
	else if (glfwGetKey(_window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		if (isPressed == 0) {
			isPressed = 5;
			isPaused = !isPaused;
		}
	}
	else isPressed = 0;

	clampOffset();
	return;
}

void initWindow(GLFWwindow*& _window) {
	if (!glfwInit()) {
		fprintf(stderr, "Couldn't initialize glfw!\n");
		throw std::runtime_error("Couldn't initialize glfw!\n");
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	_window = glfwCreateWindow(windowSizeX_c, windowSizeY_c, appName_c, NULL, NULL);
	if (!_window) {
		fprintf(stderr, "Couldn't create window!\n");
		throw std::runtime_error("Couldn't create window\n");
	}
	glfwMakeContextCurrent(_window);
	glfwSetMouseButtonCallback(_window, mousePressCallback);
	glfwSetScrollCallback(_window, scrollCallback);
	return;
}

void dispatchArgs(int32_t _argc, char** _argv) {
	std::unordered_map<std::string, uint32_t> map;
	map.insert({ "-h", 0 }); map.insert({ "-help", 0 });
	map.insert({ "-s", 1 }); map.insert({ "-size", 1 });
	map.insert({ "-d", 2 }); map.insert({ "-density", 2 });
	map.insert({ "-seed", 3 });

	size_t i = 0;
	while (i < _argc) {
		if (map.find(_argv[i]) == map.end()) {
			i++;
			continue;
		}
		switch (map[_argv[i]]) {
			case 0: {
				printf("%s\n", helpMessage);
				throw std::runtime_error("end!");
			} break;
			case 1: {
				mapSize = std::atoi(_argv[i + 1]);
				i += 2;
			} break;
			case 2: {
				density = std::atoi(_argv[i + 1]);
				i += 2;
			} break;
			case 3: {
				seed = std::atoi(_argv[i + 1]);
				i += 2;
			} break;
			default: {
				printf("%s", _argv[i]);
				i++;
			}
		};
	}
	return;
}

int32_t main(int32_t _argc, char** _argv) {
	GLFWwindow* window;
	mapSize = 1024;
	density = 50;
	srand(0);
	seed = rand();
	dispatchArgs(_argc, _argv);

	initWindow(window);
	glewInit();

	std::array<vertex_t, 4> vertices;
	vertices[0] =	{ -1.0, -1.0,	0.0,	0.0 };
	vertices[1] =	{ 1.0,	 -1.0,		1.0,	0.0 };
	vertices[2] =	{ 1.0,	 1.0,		1.0,	1.0 };
	vertices[3] =	{ -1.0, 1.0,		0.0,	1.0 };
	std::array<uint32_t, 6> indices; 
	indices = { 0,1,2,2,3,0 };

	uint32_t vao, vbo, ibo;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(vertex_t), vertices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const void*)(offsetof(vertex_t, vertex_t::texCoords)) );

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	uint32_t prevState, curState;
	glGenTextures(1, &prevState);
	glBindTexture(GL_TEXTURE_2D, prevState);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, mapSize, mapSize, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glGenTextures(1, &curState);
	glBindTexture(GL_TEXTURE_2D, curState);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, mapSize, mapSize, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(0, prevState, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(1, curState, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	currStateTexId = curState;
	prevStateTexId = prevState;

	uint32_t shaderProgram = createShaderProgram({ GL_VERTEX_SHADER, GL_FRAGMENT_SHADER }, {"res/vertex.glsl", "res/fragment.glsl"});
	glUseProgram(shaderProgram);
	glUniform1f(glGetUniformLocation(shaderProgram, "mapSize"), float(mapSize));

	uint32_t computeProgram = createShaderProgram({ GL_COMPUTE_SHADER }, { "res/compute.glsl" });
	glUseProgram(computeProgram);
	glUniform1i(glGetUniformLocation(computeProgram, "isFirst"), int32_t(true));
	glUniform1f(glGetUniformLocation(computeProgram, "mapSize"), float(mapSize));
	glUniform1ui(glGetUniformLocation(computeProgram, "seed"), uint32_t(seed));
	glUniform1ui(glGetUniformLocation(computeProgram, "density"), uint32_t(density));

	glDispatchCompute(mapSize, mapSize, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	
	glUniform1i(glGetUniformLocation(computeProgram, "isFirst"), int32_t(false));
	glCopyImageSubData(curState, GL_TEXTURE_2D, 0, 0, 0, 0, prevState, GL_TEXTURE_2D, 0, 0, 0, 0, mapSize, mapSize, 1);

	glClearColor(1, 1, 1, 1);
	while (!glfwWindowShouldClose(window)) {
		processInput(window);
		if (!isPaused) {
			glUseProgram(computeProgram);
			glDispatchCompute(mapSize, mapSize, 1);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			glCopyImageSubData(curState, GL_TEXTURE_2D, 0, 0, 0, 0, prevState, GL_TEXTURE_2D, 0, 0, 0, 0, mapSize, mapSize, 1);
		}

		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(shaderProgram);
		glUniform1f(glGetUniformLocation(shaderProgram, "zoom"), zoom);
		glUniform2f(glGetUniformLocation(shaderProgram, "cordsOffset"), offsetX, offsetY);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, curState);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

		glfwSwapBuffers(window);
		glfwPollEvents();
	 }

	return EXIT_SUCCESS;
}