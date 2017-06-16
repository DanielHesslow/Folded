#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal_modelspace;

out vec2 uv;
out vec3 pos;
out vec3 normal;

uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform mat4 M_IT;
uniform vec3 LightPosition_worldspace;
uniform float normal_mul;


void main(){

	gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
	pos = (M * vec4(vertexPosition_modelspace,1)).xyz;
	normal = ( M_IT * vec4(vertexNormal_modelspace,0)).xyz*normal_mul; 
	uv = vertexUV;
}

