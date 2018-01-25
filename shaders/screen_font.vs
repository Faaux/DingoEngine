#version 440

layout(location = 0) in vec2 screenPosition;
layout(location = 1) in vec2 uvIn;

uniform vec2 screenSize;

out vec2 uv;

void main()
{
	vec2 nds = (screenPosition/screenSize) * 2 - vec2(1);
    gl_Position = vec4(nds, 0, 1.0);
	uv = uvIn;
}
