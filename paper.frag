#version 330 core

// Interpolated values from the vertex shaders
in vec2 uv;
in vec3 pos;
in vec3 normal;
layout (location = 0) out vec4 color;
uniform vec3 in_color;

void main(){
	float alias_w = 0.007;
	float white_w = 0.05;
	float black_w = 0.000;
	float dy = abs(white_w/2-mod(uv.y,white_w));
	float dx = abs(white_w/2-mod(uv.x,white_w));
	float d = min(dy,dy);
	vec3 o = mix(vec3(0.4f),in_color,smoothstep(black_w-alias_w/2,black_w+alias_w/2,d));
	color = vec4(o,1.0);
}