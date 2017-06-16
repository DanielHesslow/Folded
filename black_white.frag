#version 330 core

// Interpolated values from the vertex shaders
in vec2 uv;
in vec3 pos;
in vec3 normal;
layout (location = 0) out vec4 color;
uniform int is_back;
uniform vec3 background;
uniform sampler2D texture;

void main(){

	vec3 tex = texture(texture,uv).rgb;
	float f = tex.r*0.2126+tex.g*0.7152+tex.b*0.0722;

	color = vec4(vec3(f),1.0);
}