#version 450

layout(location = 0) uniform float zoom;
layout(location = 1) uniform vec2 cordsOffset;
layout(location = 2) uniform float mapSize;

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoords;

layout(location = 0) out vec2 v_texCoords;

void main(void) {
	v_texCoords = clamp(texCoords/zoom + cordsOffset/mapSize, 0, 1);
	gl_Position = vec4(position, 0.0, 1.0);

	return;
}