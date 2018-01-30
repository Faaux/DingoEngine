#version 440

uniform vec3 lightColor;

in vec3 frag_colors;

in vec3 view_normal;
in vec3 view_light_pos;
in vec3 view_pos;

out vec4 out_color;

void main()
{
    vec3 norm = view_normal;
    vec3 lightDir = normalize(view_light_pos - view_pos);
    vec3 viewDir = normalize(-view_pos);

    // Ambient Light
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse Light
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular Light
    vec3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = 0.3 * spec * lightColor;

    vec3 result = (specular + diffuse + ambient) * frag_colors;
    out_color = vec4(result, 1.0f);
    //out_color = vec4(frag_colors,1);
}
