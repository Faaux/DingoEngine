#version 440

layout(location = 0) in vec2 screenPosition;
layout(location = 1) in vec2 uvIn;

uniform vec2 screenSize;
uniform vec2 ndsPosition;

out vec2 uv;

void main()
{
    vec2 nds = screenPosition / screenSize * 2;
    gl_Position = vec4(ndsPosition + nds, 0, 1.0);
    uv = uvIn;
}
