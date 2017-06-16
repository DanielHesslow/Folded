#version 330 core

in vec2 uv;
in vec3 pos;
in vec3 normal;

out vec4 color;

uniform sampler2D texture;

uniform mat4 MV;
uniform mat4 M;
uniform vec3 LightPosition_worldspace;
uniform float uvx_scale;

void main(){
	
	
	vec3 LightColor = vec3(1,1,1);
	float LightPower = 1.0f;
	
	vec2 uuv = (M*vec4(uv.x*uvx_scale,uv.y,0,1)).xy;
	
	
	vec4 tex = texture( texture, uuv);
	float alpha =  tex.a;

	
	float distance = length( LightPosition_worldspace - pos);

	vec3 n = normalize( normal);
	vec3 l = normalize( LightPosition_worldspace-pos);
	
	float cosTheta = dot( n,l ); 
	float cosFactor = clamp(cosTheta,0.0,1.0);
	
	// note that we're outputting undef color if alpha == 0
	// however when alpha == 0, we won't use it anyway 
	// so no final result is undefined and ogl can't crash on div by zero or anything so we're all good 
	vec3 o = tex.rgb/vec3(alpha); 

	o = o * clamp(cosFactor / (distance *distance) * LightPower,0.2,1.0);
	color = vec4(o,alpha);
}


















