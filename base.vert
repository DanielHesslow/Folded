#version 330 core

layout(location = 0) in vec3 pos_ms;
layout(location = 1) in vec2 uv_in;
layout(location = 2) in vec3 normal_ms;

out vec2 uv;
out vec3 pos;
out vec3 normal;

uniform mat4 MVP;
uniform mat4 M;
uniform mat4 M_IT;

void main(){
	gl_Position =  MVP * vec4(pos_ms,1);
	pos = (M * vec4(pos_ms,1)).xyz;
	normal = ( M_IT * vec4(normal_ms,0)).xyz; 
	uv = uv_in;
}

