#version 440

in  vec3 v_Color;
out vec4 out_FragColor;

void main()
{
    out_FragColor = vec4(v_Color,1);
};