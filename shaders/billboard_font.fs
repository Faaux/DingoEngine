#version 440

in vec2 uv;
uniform sampler2D texSampler;

out vec4 out_FragColor;

void main()
{
    float value = texture(texSampler, uv).r;

    out_FragColor = vec4(value) * vec4(1,0,0,1);
    //out_FragColor = vec4(1);
}
