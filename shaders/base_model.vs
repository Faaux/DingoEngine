#version 440

layout(location = 0) in vec3 vp;
layout(location = 1) in vec3 colors;

uniform mat4 modViewProj;

out vec3 frag_colors;

void main(){

    gl_Position = modViewProj * vec4(vp, 1.0);
    frag_colors = vec3(1);
}
