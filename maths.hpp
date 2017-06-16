#pragma once


vec2 get_mousepos_in_ws(mat4 projection_matrix, mat4 view_matrix, float z) {

	double mouse_x, mouse_y;
	int w, h;
	glfwGetCursorPos(window, &mouse_x, &mouse_y);
	glfwGetWindowSize(window, &w, &h);

	float x = (float)((2.0f * mouse_x) / w - 1.0f);
	float y = (float)(1.0f - (2.0f * mouse_y) / h);
	vec4 ray_clip = vec4(x, y, -1.0, 1.0);
	vec4 ray_eye = inverse(projection_matrix) * ray_clip;
	ray_eye.z = -1.0f; ray_eye.w = 0.0f;
	vec3 ray_world = normalize(vec3((inverse(view_matrix) * ray_eye)));
	vec4 o = inverse(view_matrix)*vec4(0, 0, 0, 1);
	float t = (z - o.z) / ray_world.z;
	return vec2(o.x + ray_world.x*t, o.y + ray_world.y*t);
}


mat4 basis(vec2 x_axis, vec2 origo) {
	mat4 rot = glm::mat4(
		x_axis.x, x_axis.y, 0.0f, 0,
		-x_axis.y, x_axis.x, 0.0f, 0,
		0.0f, 0.0f, 1.0f, 0.0f,
		origo.x, origo.y, 0.0f, 1.0f
	);

	return rot;
}

vec3 lerp(vec3 a, vec3 b, float t) {
	return a *(1.0f - t) + b * t;
}

vec2 lerp(vec2 a, vec2 b, float t) {
	return a *(1.0f - t) + b * t;
}

float lerp(float a, float b, float t) {
	return a *(1.0f - t) + b * t;
}

VertexInfo lerp(VertexInfo a, VertexInfo b, float t) {
	VertexInfo ret;
	ret.normal = lerp(a.normal, b.normal, t);
	ret.uv = lerp(a.uv, b.uv, t);
	ret.vert = lerp(a.vert, b.vert, t);
	return ret;
}

float area(VertexInfo a, VertexInfo b, VertexInfo c) {
	vec3 tmp = cross(a.vert - b.vert, a.vert - c.vert);
	return length(tmp);
}


float signed_area(vec2 a, vec2 b, vec2 c) {
	// return 2x area of abc if abc is clockwise otherwise -2x abc 
	return cross(vec3(c - a, 0), vec3(b - a, 0)).z;
}


float area_quad(vec2 a, vec2 b, vec2 c, vec2 d) {
	float fst = signed_area(a, b, c);
	float snd = signed_area(b, c, d);
	if ((fst >= 0) != (snd < 0)) {
		return fabs(fst - snd);
	} else {
		float trd = signed_area(c, d, a);
		return fabs(fst + trd);
	}
}
vec2 rot(vec2 v) {
	return vec2(-v.y, v.x);
}
vec2 line_line_intersect(vec2 p1, vec2 p2, vec2 p3, vec2 p4, bool *intersect) {
	float x1, x2, x3, x4, y1, y2, y3, y4;
	x1 = p1.x; x2 = p2.x; x3 = p3.x; x4 = p4.x;
	y1 = p1.y; y2 = p2.y; y3 = p3.y; y4 = p4.y;


	float xtop = (x1*y2 - y1*x2)*(x3 - x4) - (x3*y4 - y3*x4)*(x1 - x2);
	float ytop = (x1*y2 - y1*x2)*(y3 - y4) - (x3*y4 - y3*x4)*(y1 - y2);
	float bott = (x1 - x2)*(y3 - y4) - (y1 - y2)*(x3 - x4);
	*intersect = bott != 0;
	return vec2(xtop / bott, ytop / bott);
}

int floor_to_int(float f, float *rem) {
	int ret = f;
	*rem = f - ret;
	return ret;
}

float floor(float f, float *rem) {
	float ret = floor(f);
	*rem = f - ret;
	return ret;
}

int round_to_int(float f, float *rem) {
	int ret = round(f);
	*rem = f - ret;
	return ret;
}

float round(float f, float *rem) {
	float ret = round(f);
	*rem = f - ret;
	return ret;
}

int ceil_to_int(float f, float *rem) {
	int ret = ceil(f);
	*rem = f - ret;
	return ret;
}

float ceil(float f, float *rem) {
	float ret = ceil(f);
	*rem = f - ret;
	return ret;
}
