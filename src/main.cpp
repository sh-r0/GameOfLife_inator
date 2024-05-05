#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <array>
#include <vector>
#include <cassert>

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

constexpr uint32_t windowSizeX_c = 1024, windowSizeY_c = 1024;
constexpr const char* appName_c = "GameOfLife_inator";

float zoom = 1.0f;
float offsetX = 0, offsetY= 0;
size_t mapSize;
bool isPaused = false;

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

int32_t isPressed = 0;
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
	glfwSetScrollCallback(_window, scrollCallback);
	return;
}

int32_t main(int32_t _argc, char** _argv) {
	//if (_argc < 2)
	//	return EXIT_FAILURE;
	//
	//mapSize = atoi(_argv[1]);
	mapSize = 2048;
	GLFWwindow* window;
	initWindow(window);
	glewInit();

	std::array<vertex_t, 4> vertices;
	vertices[0] =	{ -1.0, -1.0,	0.0,	0.0 };
	vertices[1] =	{ 1.0,	 -1.0,		1.0,	0.0 };
	vertices[2] =	{ 1.0,	 1.0,		1.0,	1.0 };
	vertices[3] =	{ -1.0, 1.0,		0.0,	1.0 };
	std::array<uint32_t, 6> indices = { 0,1,2,2,3,0 };

	uint32_t vao, vbo, ibo;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(vertex_t), vertices.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const void*)(offsetof(vertex_t, vertex_t::texCoords)) );

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);

	uint32_t prevState, curState;
	glGenTextures(1, &prevState);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, prevState);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, mapSize, mapSize, 0, GL_RGBA, GL_FLOAT, NULL);

	glGenTextures(1, &curState);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, curState);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, mapSize, mapSize, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(0, curState, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	uint32_t shaderProgram = createShaderProgram({ GL_VERTEX_SHADER, GL_FRAGMENT_SHADER }, {"res/vertex.glsl", "res/fragment.glsl"});
	glUseProgram(shaderProgram);
	glUniform1f(glGetUniformLocation(shaderProgram, "mapSize"), float(mapSize));

	uint32_t computeProgram = createShaderProgram({ GL_COMPUTE_SHADER }, { "res/compute.glsl" });
	glUseProgram(computeProgram);
	glUniform1i(glGetUniformLocation(computeProgram, "prevState"), 0);
	glUniform1i(glGetUniformLocation(computeProgram, "isFirst"), true);
	glUniform1f(glGetUniformLocation(computeProgram, "mapSize"), float(mapSize));
	glDispatchCompute(mapSize, mapSize, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	
	glUniform1i(glGetUniformLocation(computeProgram, "isFirst"), false);
	glCopyImageSubData(curState, GL_TEXTURE_2D, 0, 0, 0, 0, prevState, GL_TEXTURE_2D, 0, 0, 0, 0, mapSize, mapSize, 1);

	//glUniform1i(glGetUniformLocation(shaderProgram, "curState"), 1);
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
		//glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(vertex_t), vertices.data());
		//glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(uint32_t), indices.data());
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

		glfwSwapBuffers(window);
		glfwPollEvents();
	 }

	return EXIT_SUCCESS;
}