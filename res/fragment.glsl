#version 450

layout(location = 3) uniform sampler2D curState;

layout(location = 0) in vec2 v_texCoords;

layout(location = 0) out vec4 outColor;

void main(void) {
	outColor = vec4(vec3(texture(curState, v_texCoords).r), 1.0f);
	//outColor = vec4(1,0,1,1);
	return;
}