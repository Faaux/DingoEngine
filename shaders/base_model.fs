#version 440

uniform vec3 lightColor;

in vec3 frag_colors;

in vec3 view_normal;
in vec3 view_light_dir;
in vec3 view_pos;
in vec3 lightSpacePosition;

out vec4 out_color;

uniform float bias;
uniform vec2 resolution;
uniform sampler2DShadow texSampler;

void main()
{
    // Ambient Light
    float ambientStrength = 0;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = view_normal;
    vec3 lightDir = normalize(-view_light_dir);
    vec3 viewDir = normalize(-view_pos);

    vec2 poissonDisk[4] = vec2[](vec2(-0.94201624, -0.39906216), vec2(0.94558609, -0.76890725),
                                 vec2(-0.094184101, -0.92938870), vec2(0.34495938, 0.29387760));

    float total = 0;
    for (int i = 0; i < 4; i++)
    {
        vec2 newUv = lightSpacePosition.xy + poissonDisk[i] / resolution;

        total = total + texture(texSampler, vec3(newUv, lightSpacePosition.z - bias));
    }
    float shadowFactor = total / 4.0;

    // Diffuse Light
    float cosTheta = dot(norm, lightDir);
    float diff = max(cosTheta, 0.0);
    vec3 diffuse = diff * lightColor * shadowFactor;

    // Specular Light
    vec3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = 0.3 * spec * lightColor * shadowFactor;

    vec3 result = (diffuse + ambient) * frag_colors;
    out_color = vec4(result, 1.0f);
    // out_color = vec4(vec3(depth),1);
}
