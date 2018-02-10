#version 440

layout(location = 0) in vec3 position;

uniform mat4 vp;
uniform mat4 m;

void main() { gl_Position = vp * m * vec4(position, 1); }
