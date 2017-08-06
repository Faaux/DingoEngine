#version 440

uniform vec4 inputColor;
in vec3 frag_colors;

void main(){
	gl_FragColor = vec4(frag_colors, 1.0f);
	// gl_FragColor = inputColor;
			
}