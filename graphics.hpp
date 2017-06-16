#pragma once

/*
	Here goes graphics API simplifications. ie load shader, draw mesh etc.
*/


#define printOpenGLError() printOglError(__FILE__, __LINE__)
int printOglError(char *file, int line) {

	GLenum glErr;
	int    retCode = 0;

	while ((glErr = glGetError()) != GL_NO_ERROR) {
		printf("glError in file %s @ line %d: %s\n",
			file, line, gluErrorString(glErr));
		retCode = 1;
	}
	return retCode;
}

char *m_read_file(char *file_path) {
	FILE *f = fopen(file_path, "rb");
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *mem = (char *)malloc(fsize + 1);
	fread(mem, 1, fsize, f);
	mem[fsize] = '\0';
	return mem;
}


GLuint LoadShaders(char *vertex_file_path, char *fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);


	char *vertex_shader_source = m_read_file(vertex_file_path);
	char *fragment_shader_source = m_read_file(fragment_file_path);

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	glShaderSource(VertexShaderID, 1, &vertex_shader_source, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 1) {
		char *err = (char *)alloca(InfoLogLength);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, err);
		printf("%s\n", err);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	glShaderSource(FragmentShaderID, 1, &fragment_shader_source, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 1) {
		char *err = (char *)alloca(InfoLogLength);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, err);
		printf("%s\n", err);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 1) {
		char *err = (char *)alloca(InfoLogLength);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, err);
		printf("%s\n", err);
	}

	if (!Result)__debugbreak();

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	free(vertex_shader_source);
	free(fragment_shader_source);

	return ProgramID;
}

void mesh_update(Mesh *m) {
	glBindVertexArray(m->vao_id);
	glBindBuffer(GL_ARRAY_BUFFER, m->vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, m->verts.length * sizeof(glm::vec3), &m->verts[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, m->uv_buffer);
	glBufferData(GL_ARRAY_BUFFER, m->uvs.length * sizeof(glm::vec2), &m->uvs[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, m->normal_buffer);
	glBufferData(GL_ARRAY_BUFFER, m->normals.length * sizeof(glm::vec3), &m->normals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->indices.length * sizeof(unsigned short), &m->indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m->vertex_buffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, m->uv_buffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, m->normal_buffer);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->index_buffer);

}

void mesh_init(Mesh *m) {
	glGenVertexArrays(1, &m->vao_id);
	glBindVertexArray(m->vao_id);
	glGenBuffers(1, &m->vertex_buffer);
	glGenBuffers(1, &m->uv_buffer);
	glGenBuffers(1, &m->normal_buffer);
	glGenBuffers(1, &m->index_buffer);
	mesh_update(m);
}

void mesh_destroy(Mesh *m) {
	glDeleteBuffers(1, &m->vertex_buffer);
	glDeleteBuffers(1, &m->uv_buffer);
	glDeleteBuffers(1, &m->normal_buffer);
	glDeleteBuffers(1, &m->index_buffer);
	glDeleteVertexArrays(1, &m->vao_id);
}

void mesh_draw(Mesh *m, DrawMode draw_mode = draw_mode_solid) {
	glBindVertexArray(m->vao_id);
	GLenum mode = draw_mode == draw_mode_solid ? GL_TRIANGLES : (draw_mode == draw_mode_points ? GL_POINTS : GL_LINES);
	glDrawElements(mode, (GLsizei)m->indices.length, GL_UNSIGNED_SHORT, (void*)0);
}

void mesh_builder_add(MeshBuilder *mb, int a, int b, int c) {
	mb->mesh->indices.add(a);
	mb->mesh->indices.add(b);
	mb->mesh->indices.add(c);
}

int mesh_builder_add(MeshBuilder *mb, VertexInfo info) {
	int index;
	if (mb->map.lookup(info, &index)) {
		mb->mesh->indices.add(index);
		return index;
	} else {
		unsigned short idx = (unsigned short)mb->mesh->verts.length;
		mb->mesh->indices.add(idx);
		mb->mesh->verts.add(info.vert);
		mb->mesh->normals.add(info.normal);
		mb->mesh->uvs.add(info.uv);
		mb->map.insert(info, idx);
		return idx;
	}
}

void mesh_builder_add(MeshBuilder *mb, VertexInfo a, VertexInfo b, VertexInfo c) {
	mesh_builder_add(mb, a);
	mesh_builder_add(mb, b);
	mesh_builder_add(mb, c);
}


GLuint make_fbo(GLuint *texture, MRT_Settings settings = {}) {
	printOpenGLError();
	if (settings.size == 0) settings.size = 1024;

	// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
	GLuint Framebuffer_id = 0;
	glGenFramebuffers(1, &Framebuffer_id);
	glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer_id);

	// The texture we're going to render to
	GLuint renderedTexture;
	glGenTextures(1, &renderedTexture);
	GLint filter = settings.nearest ? GL_NEAREST : GL_LINEAR;
	// "Bind" the newly created texture : all future texture functions will modify this texture
	if (settings.samples > 1) {
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, renderedTexture);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, settings.samples, GL_RGBA, settings.size, settings.size, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, renderedTexture, 0);
	} else {
		glBindTexture(GL_TEXTURE_2D, renderedTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, settings.size, settings.size, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture, 0);
	}

	// Set "renderedTexture" as our colour attachement #0

	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) __debugbreak();
	*texture = renderedTexture;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	printOpenGLError();
	return Framebuffer_id;
}


Mesh gen_quad(vec2 center = vec2(0.5)) {
	Mesh ret = Mesh::make();
	MeshBuilder builder;
	builder.map = IndexFromVertex::make(16);
	builder.mesh = &ret;

	vec3 cc = vec3(center.x, center.y, 0.0f);
	// dc
	// ab
	VertexInfo a = { vec3(-0.5,-0.5,0) + cc,vec3(0,0,1),vec2(0,0) };
	VertexInfo b = { vec3(0.5,-0.5,0) + cc,vec3(0,0,1),vec2(1,0) };
	VertexInfo c = { vec3(0.5,0.5,0) + cc,vec3(0,0,1),vec2(1,1) };
	VertexInfo d = { vec3(-0.5,0.5,0) + cc,vec3(0,0,1),vec2(0,1) };

	mesh_builder_add(&builder, a, c, b);
	mesh_builder_add(&builder, a, d, c);

	builder.map.destroy();
	mesh_init(&ret);
	return ret;
}





struct BezierSegment {
	vec2 a, b, c, d;
};

vec2 eval(BezierSegment bs, float t, vec2 *derivative) {
	vec2 abt = lerp(bs.a, bs.b, t);
	vec2 bct = lerp(bs.b, bs.c, t);
	vec2 cdt = lerp(bs.c, bs.d, t);

	vec2 abct = lerp(abt, bct, t);
	vec2 bcdt = lerp(bct, cdt, t);

	vec2 abcdt = lerp(abct, bcdt, t);
	*derivative = bcdt - abct;
	return abcdt;
}

BezierSegment segment_from_index(DA_vec2 *points, int first_point_index) 	{
	int i = first_point_index;
	BezierSegment seg;
	seg.a = (*points)[i];
	seg.b = (*points)[i + 1];
	seg.c = (*points)[i + 2];
	seg.d = (*points)[(i + 3) % points->length];
	return seg;
}

void split(BezierSegment bs, BezierSegment *_out_a, BezierSegment *_out_b, float t) {
	vec2 abt = lerp(bs.a, bs.b, t);
	vec2 bct = lerp(bs.b, bs.c, t);
	vec2 cdt = lerp(bs.c, bs.d, t);

	vec2 abct = lerp(abt, bct, t);
	vec2 bcdt = lerp(bct, cdt, t);

	vec2 abcdt = lerp(abct, bcdt, t);
	_out_a->a = bs.a;
	_out_a->b = abt;
	_out_a->c = abct;
	_out_a->d = abcdt;

	_out_b->a = abcdt;
	_out_b->b = bcdt;
	_out_b->c = cdt;
	_out_b->d = bs.d;
}

void split(DA_vec2 *beiz_points, float t) {
	int num_segments = beiz_points->length/3;
	float sub_t;
	int seg_index = floor_to_int(num_segments*t, &sub_t);
	BezierSegment seg = segment_from_index(beiz_points, seg_index * 3);
	BezierSegment a, b;
	split(seg, &a, &b, sub_t);

	beiz_points->removeOrd(seg_index * 3 + 1);
	beiz_points->removeOrd(seg_index * 3 + 1);

	beiz_points->insert(a.b, seg_index * 3 + 1);
	beiz_points->insert(a.c, seg_index * 3 + 2);
	beiz_points->insert(a.d, seg_index * 3 + 3);
	beiz_points->insert(b.b, seg_index * 3 + 4);
	beiz_points->insert(b.c, seg_index * 3 + 5);

}

void gen_segment(MeshBuilder *builder, BezierSegment seg, float width, float t_start, float t_end) {

	// NOTE uv in y direction (t_start, t_end) is currently OFF!

	vec2 ab = seg.b - seg.a;
	vec2 norm_ad = normalize(seg.d - seg.a);
	vec2 cd = seg.d - seg.c;

	float reject_ab = length(ab - ab*dot(ab, norm_ad));
	float reject_cd = length(cd - cd*dot(cd, norm_ad));

	if (reject_ab + reject_cd < 0.01) {

		// c d
		// a b

		// d
		// a

		VertexInfo a, b, c, d;
		a.normal = vec3(0, 0, 1);
		b.normal = vec3(0, 0, 1);
		c.normal = vec3(0, 0, 1);
		d.normal = vec3(0, 0, 1);
		vec2 tan_a = normalize(ab);
		vec2 tan_d = normalize(cd);
		vec2 orth_a = vec2(-tan_a.y, tan_a.x);
		vec2 orth_d = vec2(-tan_d.y, tan_d.x);

		a.vert = vec3(seg.a + width * orth_a, 0);
		b.vert = vec3(seg.a - width * orth_a, 0);
		c.vert = vec3(seg.d + width * orth_d, 0);
		d.vert = vec3(seg.d - width * orth_d, 0);

		a.uv = vec2(0, t_start);
		b.uv = vec2(1, t_start);
		c.uv = vec2(0, t_end);
		d.uv = vec2(1, t_end);

		mesh_builder_add(builder, a, b, c);
		// NOTE we rely on that D is the last added for collisions. (that, [0,1,2,4] is the polygon), and (a,c) and (b,d) are the sides
		mesh_builder_add(builder, c, d, b);

	} else {
		BezierSegment a, b;
		split(seg, &a, &b, 0.5);
		gen_segment(builder, a, width, t_start, (t_start + t_end) / 2);
		gen_segment(builder, b, width, (t_start + t_end) / 2, t_end);
	}



}

Mesh gen_beizpath(DA_vec2 points, float width) {
	assert(points.length % 3 == 0);
	MeshBuilder builder;
	Mesh m = Mesh::make();
	builder.map = IndexFromVertex::make(16);
	builder.mesh = &m;

	for (int i = 0; i < points.length; i += 3) {
		BezierSegment seg = segment_from_index(&points,i);
		//uvs off. should aprox length of curve to get it better! (doesn't really matter though for what we do)
		float ts = i / (float)(points.length);
		float tm = (i + 1.5) / (points.length);
		float te = (i + 3.0) / (points.length);
		BezierSegment a, b;
		split(seg, &a, &b, 0.5);

		gen_segment(&builder, a, width, ts, tm);
		gen_segment(&builder, b, width, tm, te);
	}
	mesh_init(&m);
	return m;
}




Mesh gen_taco() {
	Mesh ret = Mesh::make();

#define RES_X 25
#define RADIUS 0.002
	float start_bend = (float)((1.0 / (RADIUS*PI + 2.0)));
	float end_bend = (float)((1.0 + RADIUS*PI) / (RADIUS*PI + 2.0));

	ret.normals.add(vec3(0, 0, 1));
	ret.normals.add(vec3(0, 0, 1));
	ret.uvs.add(vec2(0, 0));
	ret.uvs.add(vec2(0, 1));
	ret.verts.add(vec3(0, 0, 0));
	ret.verts.add(vec3(0, 1, 0));

	for (int ix = 0; ix < RES_X; ix++) {
		float t = ix / ((float)RES_X - 1);
		float x;
		float z;
		vec3 normal;

		x = (float)(sin(t*PI)*RADIUS + 1.0f);
		z = (float)(cos(t*PI)*RADIUS - RADIUS);

		float nx = (float)-sin(t*PI);
		float nz = (float)cos(t*PI);

		normal = vec3(nx, 0, nz);

		int idx_prev = ix * 2 + 2;
		for (int iy = 0; iy < 2; iy++) {
			float y = iy / ((float)2 - 1);
			ret.verts.add(vec3(x, y, z));
			ret.normals.add(normal);
			ret.uvs.add(vec2(t*(end_bend - start_bend) + start_bend, y));
		}

		if (idx_prev != 0) {
			for (int iy = 0; iy < 2 - 1; iy++) {
				int a = idx_prev + iy + 1 - 2;
				int b = idx_prev + iy + 1;
				int c = idx_prev + iy;
				int d = idx_prev + iy - 2;

				ret.indices.add(a);
				ret.indices.add(b);
				ret.indices.add(c);

				ret.indices.add(a);
				ret.indices.add(c);
				ret.indices.add(d);
			}
		}
	}

	int idx_prev = RES_X * 2 + 2;
	ret.normals.add(vec3(0, 0, -1));
	ret.normals.add(vec3(0, 0, -1));
	ret.uvs.add(vec2(1, 0));
	ret.uvs.add(vec2(1, 1));
	ret.verts.add(vec3(0, 0, -2 * RADIUS));
	ret.verts.add(vec3(0, 1, -2 * RADIUS));

	for (int iy = 0; iy < 2 - 1; iy++) {
		int a = idx_prev + iy + 1 - 2;
		int b = idx_prev + iy + 1;
		int c = idx_prev + iy;
		int d = idx_prev + iy - 2;

		ret.indices.add(a);
		ret.indices.add(b);
		ret.indices.add(c);

		ret.indices.add(a);
		ret.indices.add(c);
		ret.indices.add(d);
	}
	mesh_init(&ret);
	return ret;
}


