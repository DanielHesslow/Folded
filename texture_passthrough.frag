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
	color = texture(texture,uv);
}