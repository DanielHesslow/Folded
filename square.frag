#version 330 core

// Interpolated values from the vertex shaders
in vec2 uv;
in vec3 pos;
in vec3 normal;
layout (location = 0) out vec4 color;
uniform vec3 in_color;
float f_width(float f)
{
    return pow((pow(dFdx(f),2.0) + pow(dFdy(f),2.0)),0.5);
}



vec4 render(float d, vec3 color, float stroke)
{
    float fw = f_width(d);
    float anti = fw * 5.0;
    float fw_stroke = fw*stroke;
    vec4 strokeLayer = vec4(vec3(0.05), 1.0-smoothstep(-anti, 0, d ));
    vec4 colorLayer = vec4(color, 1.0-smoothstep(-anti, 0, d+fw_stroke));
    return  vec4(mix(strokeLayer.rgb, colorLayer.rgb, colorLayer.a), strokeLayer.a);
}




void main(){
    vec2 dc = abs(vec2(0.5)-uv);
    float d = max(dc.x,dc.y)-0.45f;
    vec4 cx = render(dc.y-0.5,in_color,2f);
    vec4 cy = render(dc.x-0.5,in_color,2f);
    if(cx.a < cy.a){
        color = cx;
    } else {
        color = cy;
    }
	color.rgb *= vec3(color.a);
}
