#version 450
layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D curState;
layout(location= 0) uniform float mapSize;
layout(location= 1) uniform sampler2D prevState;  
layout(location= 2) uniform bool isFirst;

float rand(vec2 co) {
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

bool isAlive(void) {
    const ivec2 offsets[8] = {
        ivec2(1,-1),    ivec2(1,0),     ivec2(1,1),
        ivec2(0,-1),                         ivec2(0,1),
        ivec2(-1,-1),   ivec2(-1,0),    ivec2(-1,1) 
    };
    uint living = 0;
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    for(int i = 0; i < 8; i++)
        if(texture(prevState, (texelCoord+offsets[i])/mapSize).r == 0) living++;
    
    if(living == 3) return true;
    else if(living == 2 && texture(prevState, texelCoord/mapSize).r == 0) return true;
    
    return false;
}

void main(void) {
    const int seed = 1, density = 50;
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    if(isFirst) {
        float val;
        if(rand(texelCoord + rand(ivec2(seed))) > 1 - density/100.0f)
            val = 0.0f;
        else 
            val = 1.0f;
        imageStore(curState, texelCoord, vec4(val)); 
    } else {
        if(isAlive()) imageStore(curState, texelCoord, vec4(0.0f));
        else imageStore(curState, texelCoord, vec4(1.0f));
    }
    return;
}