#version 440

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 uv;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;
uniform mat4 lightMVP;
uniform vec3 lightDirection;

out vec3 frag_colors;
out vec3 view_normal;
out vec3 view_light_dir;
out vec3 view_pos;
out vec3 lightSpacePosition;

void main()
{
    vec4 viewNormal = transpose(inverse(view * model)) * vec4(norm, 1.0);
    view_normal = normalize(viewNormal.xyz);

    view_light_dir = (transpose(inverse(view)) * vec4(lightDirection, 1)).xyz;

    frag_colors = vec3(1);

    vec4 world_pos_vec4 = model * vec4(position, 1.0);

	vec4 bla = (lightMVP * world_pos_vec4);
	vec3 lightDivision = bla.xyz / bla.w;
    lightSpacePosition = (lightDivision + vec3(1,1,1)) / 2.0;

    vec4 view_pos_vec4 = view * world_pos_vec4;
    view_pos = view_pos_vec4.xyz;
    gl_Position = proj * view_pos_vec4;
}
