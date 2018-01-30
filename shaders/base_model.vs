#version 440

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 uv;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;
uniform vec3 lightPos;

out vec3 frag_colors;
out vec3 view_normal;
out vec3 view_light_pos;
out vec3 view_pos;

void main()
{
    vec4 viewNormal = transpose(inverse(view * model)) * vec4(norm, 1.0);
    view_normal = normalize(viewNormal.xyz);

    view_light_pos = (view * vec4(lightPos, 1)).xyz;

    frag_colors = vec3(1);

    vec4 view_pos_vec4 = view * model * vec4(position, 1.0);
    view_pos = view_pos_vec4.xyz;
    gl_Position = proj * view_pos_vec4;
}
