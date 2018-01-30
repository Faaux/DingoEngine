#version 440

layout(location = 0) in vec3 start;
layout(location = 1) in vec3 end;
layout(location = 2) in vec4 in_ColorPointSize;
layout(location = 3) in float direction;

out vec3 v_Color;
uniform mat4 p_Matrix;
uniform mat4 mv_Matrix;

void main()
{
	float thickness = in_ColorPointSize.w;

	vec4 startView = mv_Matrix * vec4(start, 1.0);
	vec4 endView = mv_Matrix * vec4(end, 1.0);

	vec4 dir = endView-startView;
	vec3 normal = normalize(cross(dir.xyz, vec3(0,0,1)) * direction) * thickness / 100.0;

	gl_Position = p_Matrix * vec4(startView.xyz + normal,1);
	v_Color = in_ColorPointSize.xyz;
};