#pragma once


/*
	Here goes graphics stuff that depend on our state.
	ie. drawing, sorting entities, etc.
*/



void sort_fold_pcs() {
	FoldPcs parts[1024][2];
	int len[2];
	// sorts in O(fn) time. which really isn't to bad. however we do shuffle a lot of mem..
	// but I'm pretty sure we could do it in linear. If we had to :) it clearly follows a pattern :)

	for (int f = 0; f < num_folds*(activated_trasforms)-0.5; f++) {
		int fold_bit = 1 << f;
		len[0] = len[1] = 0;
		for (int i = 0; i < num_fold_pcs; i++) {
			int fold = (folds_pcs[i].fold_bitmask&fold_bit) > 0;
			parts[len[fold]++][fold] = folds_pcs[i];
		}

		for (int i = 0; i < len[0]; i++) {
			folds_pcs[i] = parts[i][0];
		}

		for (int i = 0; i < len[1]; i++) {
			folds_pcs[i + len[0]] = parts[len[1] - i - 1][1];
		}
	}
}

mat4 getFoldPcsTransform(int i) {
	FoldPcs e = folds_pcs[i];
	mat4 m = mat4();
	m = scale(m, vec3(0.5, 0.5, 1.0));
	m = translate(m, vec3(0.5, 0.5, 0.0));

	float dt = 1.0f / num_folds;
	for (int i = 0; i < num_folds; i++) {
		if (e.fold_bitmask & (1 << i)) {
			float t = clamp((activated_trasforms - dt*i) / dt, 0.0f, 1.0f);
			mat4 rot = rotate(mat4(), t*PI, vec3(0, 1, 0));
			m = fold_transforms[i] * rot *inverse(fold_transforms[i]) * m;
		}
	}
	return m;
}

mat4 getFoldPcsTransform_no_transform(int i) {
	FoldPcs e = folds_pcs[i];
	mat4 m = mat4();
	float dt = 1.0f / num_folds;
	for (int i = 0; i < num_folds; i++) {
		if (e.fold_bitmask & (1 << i)) {
			mat4 rot = scale(mat4(), vec3(-1, 1, 1));
			m = fold_transforms[i] * rot *inverse(fold_transforms[i]) * m;
		}
	}
	return m;
}




void fold_at(vec2 dir, vec2 offset) {
	mat4 from_fold_basis = basis(-dir, offset);
	mat4 to_fold_basis = inverse(basis(-dir, offset));
	vec4 oo = vec4(offset, 0, 1);
	vec4 o2 = to_fold_basis*oo;

	static IndexFromVertex map_a = IndexFromVertex::make(128);
	static IndexFromVertex map_b = IndexFromVertex::make(128);

	int any_split = false;
	int pre_num_entities = num_fold_pcs;
	for (int i = 0; i < pre_num_entities; i++) {
		FoldPcs old = folds_pcs[i];
		MeshBuilder build_a;
		MeshBuilder build_b;
		Mesh ma = Mesh::make();
		Mesh mb = Mesh::make();
		map_a.clear();
		map_b.clear();
		build_a.map = map_a;
		build_a.mesh = &ma;
		build_b.map = map_b;
		build_b.mesh = &mb;

		mat4 model_fold = to_fold_basis*getFoldPcsTransform(i);


		for (int t = 0; t < old.mesh.indices.length; t += 3) {
			vec3 p[3];
			VertexInfo info[3];

			for (int i = 0; i < 3; i++) {
				p[i] = vec3(model_fold*vec4(old.mesh.verts[old.mesh.indices[t + i]], 1));
				info[i].normal = old.mesh.normals[old.mesh.indices[t + i]];
				info[i].vert = old.mesh.verts[old.mesh.indices[t + i]];
				info[i].uv = old.mesh.uvs[old.mesh.indices[t + i]];
			}

			float time[3]; bool b[3]; int num_splits = 0;
			for (int i = 0; i < 3; i++) {
				time[i] = -p[i].x / (p[(i + 1) % 3].x - p[i].x);
				b[i] = time[i] >= 0 && time[i] <= 1;
				num_splits += b[i];
			}
			if (num_splits == 1) {
				__debugbreak();
			} else if (num_splits == 2) {
				int lone = -1;
				for (int i = 0; i < 3; i++)
					if (!b[i]) lone = (i + 2) % 3;
				if (lone == -1) __debugbreak();

				int lone_p = (lone + 2) % 3;
				int lone_n = (lone + 1) % 3;
				VertexInfo i1 = lerp(info[lone], info[lone_n], time[lone]);
				VertexInfo i2 = lerp(info[lone_p], info[lone], time[lone_p]);

				if (((p[lone].x >= 0) == (p[lone_p].x >= 0)) ||
					((p[lone].x >= 0) == (p[lone_n].x >= 0)))__debugbreak();

				printf("lerp t = %f, %f\n", time[lone], time[lone_p]);

				MeshBuilder *builders[2] = { &build_a,&build_b };
				if (p[lone].x < 0.0f)std::swap(builders[0], builders[1]);
				mesh_builder_add(builders[0], info[lone], i2, i1);
				mesh_builder_add(builders[1], info[lone_n], info[lone_p], i1);
				mesh_builder_add(builders[1], info[lone_p], i1, i2);

				float old_area = area(info[0], info[1], info[2]);

				float a = area(info[lone], i2, i1);
				float b = area(info[lone_n], info[lone_p], i1);
				float c = area(info[lone_p], i1, i2);
				float new_area = a + b + c;

				vec3 fst = vec3(model_fold*vec4(i1.vert, 1));
				vec3 snd = vec3(model_fold*vec4(i2.vert, 1));

				if (abs((model_fold*vec4(i1.vert, 1)).x > 0.01) || abs((model_fold*vec4(i2.vert, 1)).x > 0.01)) __debugbreak();

				if (!(abs(old_area - new_area) < 0.01))__debugbreak();

			} else if (num_splits == 3) {
				__debugbreak();
			} else {
				bool b1 = (p[0].x >= 0.0f);
				bool b2 = (p[1].x >= 0.0f);
				bool b3 = (p[2].x >= 0.0f);

				if (b1 != b2 || b1 != b3)__debugbreak();
				if (p[0].x >= 0)
					mesh_builder_add(&build_a, info[0], info[1], info[2]);
				else
					mesh_builder_add(&build_b, info[0], info[1], info[2]);
			}


		}
		{
			mesh_destroy(&folds_pcs[i].mesh);

			if (ma.indices.length > 0 && mb.indices.length > 0) {
				any_split = true;
				mesh_init(&ma);
				mesh_init(&mb);
				folds_pcs[i].mesh = ma;
				folds_pcs[num_fold_pcs].mesh = mb;
				folds_pcs[num_fold_pcs].fold_bitmask = folds_pcs[i].fold_bitmask | (1 << num_folds);
				num_fold_pcs++;
			} else if (ma.indices.length > 0) {
				any_split = true;
				mesh_init(&ma);
				folds_pcs[i].mesh = ma;
			} else if (mb.indices.length > 0) {
				any_split = true;
				mesh_init(&mb);
				folds_pcs[i].mesh = mb;
				folds_pcs[i].fold_bitmask |= (1 << num_folds);
			}
			if (num_fold_pcs > 1024)__debugbreak();
		}
	}
	if (any_split) {
		fold_transforms[num_folds++] = from_fold_basis;
	}
}

struct SimpleShader {
	GLuint shader, m, mit, mvp, color;
};

SimpleShader make_simple_shader(char *frag) {
	SimpleShader ret = {};
	ret.shader = LoadShaders("base.vert", frag);
	ret.mvp = glGetUniformLocation(ret.shader, "MVP");
	ret.m = glGetUniformLocation(ret.shader, "M");
	ret.mit = glGetUniformLocation(ret.shader, "M_IT");
	ret.color = glGetUniformLocation(ret.shader, "in_color");
	return ret;
}

void bind_simple_shader(SimpleShader *shader, mat4 mvp, mat4 m, vec3 color) {
	glUseProgram(shader->shader);
	mat4 mit = transpose(inverse(m));
	printOpenGLError();
	glUniformMatrix4fv(shader->m, 1, GL_FALSE, &m[0][0]);
	printOpenGLError();
	glUniformMatrix4fv(shader->mvp, 1, GL_FALSE, &mvp[0][0]);
	printOpenGLError();
	glUniformMatrix4fv(shader->mit, 1, GL_FALSE, &mit[0][0]);
	printOpenGLError();
	glUniform3fv(shader->color, 1, &color[0]);
	printOpenGLError();
}


void draw_game(GLuint frame_buffer) {
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	glClearColor(0.0, 0.0, 0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	static SimpleShader shader = make_simple_shader("paper.frag");
	static SimpleShader c = make_simple_shader("circle.frag");
	static SimpleShader sq = make_simple_shader("square.frag");
	static Mesh m = gen_quad(vec2(0.0));
	static mat4 ortho = glm::ortho(0.5f, -0.5f, 0.5f, -0.5f);
	mat4 matrix = mat4(1.0);
	vec3 color = vec3(1.0, 1.0, 0.8);
	bind_simple_shader(&shader, ortho, ortho, color);
	mesh_draw(&m);


	for (int i = 0; i < num_entities; i++) {
		Entity e = entities[i];
		switch (e.type) {
		case type_circle:
			bind_simple_shader(&c, ortho*e.transform, e.transform, e.color);
			mesh_draw(e.mesh);
			break;
		case type_square:
			bind_simple_shader(&sq, ortho*e.transform, e.transform, e.color);
			mesh_draw(e.mesh);
			break;
		case type_curve:
			bind_simple_shader(&sq, ortho*e.transform, e.transform, e.color);
			mesh_draw(e.mesh);
			break;
		}
	}
}


void draw_back(GLuint frame_buffer, GLuint front_tex) {
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	glClearColor(0.0, 0.0, 0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	static SimpleShader shader = make_simple_shader("black_white.frag");
	static mat4 ortho = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f);
	mat4 matrix = mat4(1.0);
	vec3 color = vec3(1.0, 1.0, 0.6);
	bind_simple_shader(&shader, ortho, ortho, color);
	static GLuint texture_id = glGetUniformLocation(shader.shader, "texture");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, front_tex);
	glUniform1i(texture_id, 0);

	static Mesh m = gen_quad();
	mesh_draw(&m);
}

void draw_front_back(GLuint front, GLuint back, int texture_size) {
	// note we're working with premultiplied alpha here!
	// that's why we're changing the blend func etc.
	glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	sort_fold_pcs();
	glDisable(GL_CULL_FACE);

	GLint64 vp[4];
	glGetInteger64v(GL_VIEWPORT, vp);
	glViewport(0, 0, texture_size, texture_size);

	static GLuint tex;
	static GLuint fbo = make_fbo(&tex);
	draw_game(fbo);

	static GLuint tex_back;
	static GLuint fbo_back = make_fbo(&tex_back);
	draw_back(fbo_back,tex);


	static GLuint msa_front_tex, msa_back_tex;
	MRT_Settings settings = {};
	settings.samples = 8;
	settings.size = texture_size;
	static GLuint msaa_front = make_fbo(&msa_front_tex, settings);
	static GLuint msaa_back = make_fbo(&msa_back_tex, settings);

	static mat4 ortho = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f);
	static mat4 identity = glm::mat4();
	static Mesh quad = gen_quad();
	static GLuint paper_shader_id = LoadShaders("base.vert", "texture_passthrough.frag");
	static GLuint mvp_matrix_id = glGetUniformLocation(paper_shader_id, "MVP");
	static GLuint m_matrix_id = glGetUniformLocation(paper_shader_id, "M");
	static GLuint m_it_matrix_id = glGetUniformLocation(paper_shader_id, "M_IT");
	static GLuint background_id = glGetUniformLocation(paper_shader_id, "in_color");
	static GLuint texture_id = glGetUniformLocation(paper_shader_id, "texture");

	

	mat4 matrix = mat4();
	mat4 mvp = ortho * matrix;
	glUseProgram(paper_shader_id);
	glUniformMatrix4fv(m_matrix_id, 1, GL_FALSE, &matrix[0][0]);
	glUniformMatrix4fv(m_it_matrix_id, 1, GL_FALSE, &matrix[0][0]);

	GLfloat prev_clear[4];
	glGetFloatv(GL_COLOR_CLEAR_VALUE, prev_clear);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex_back);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);


	float color[] = { 1.0f,1.0f,0.8f };
	glUniform3fv(background_id, 1, color);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  // note premultipled alpha, color must be all zeros
	glBindFramebuffer(GL_FRAMEBUFFER, msaa_front);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (int i = 0; i < num_fold_pcs; i++) {
		FoldPcs e = folds_pcs[i];
		mat4 m = getFoldPcsTransform(i);
		mvp = ortho*m;
		glUniformMatrix4fv(mvp_matrix_id, 1, GL_FALSE, &mvp[0][0]);
		glUniform1i(texture_id, __popcnt(e.fold_bitmask) & 1);
		mesh_draw(&folds_pcs[i].mesh, draw_mode_solid);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, msaa_back);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	float color_2[] = { 1.0f,0.8f,0.8f };
	glUniform3fv(background_id, 1, color_2);

	for (int i = num_fold_pcs - 1; i >= 0; i--) {
		FoldPcs e = folds_pcs[i];
		mat4 m = getFoldPcsTransform(i);
		mvp = ortho*m;
		glUniformMatrix4fv(mvp_matrix_id, 1, GL_FALSE, &mvp[0][0]);
		glUniform1i(texture_id, ~__popcnt(e.fold_bitmask) & 1);
		mesh_draw(&folds_pcs[i].mesh, draw_mode_solid);
	}

	glBindFramebuffer(GL_READ_FRAMEBUFFER, msaa_front);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, front);
	glBlitFramebuffer(0, 0, texture_size, texture_size, 0, 0, texture_size, texture_size, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, msaa_back);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, back);
	glBlitFramebuffer(0, 0, texture_size, texture_size, 0, 0, texture_size, texture_size, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, 0);
	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0);

	//reset state
	glClearColor(prev_clear[0], prev_clear[1], prev_clear[2], prev_clear[3]);
	glViewport((GLsizei)vp[0], (GLsizei)vp[1], (GLsizei)vp[2], (GLsizei)vp[3]);
	glEnable((GLenum)GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	printOpenGLError();
}


void draw_taco(GLuint tex_front, GLuint tex_back) {
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);


	// Create and compile our GLSL program from the shaders
	static GLuint tacoShader = LoadShaders("taco.vert", "taco.frag");

	// Get a handle for our "MVP" uniform
	static GLuint MatrixID = glGetUniformLocation(tacoShader, "MVP");
	static GLuint ViewMatrixID = glGetUniformLocation(tacoShader, "V");
	static GLuint ModelMatrixID = glGetUniformLocation(tacoShader, "M");
	static GLuint normal_mul_ID = glGetUniformLocation(tacoShader, "normal_mul");
	static GLuint uvx_scale_ID = glGetUniformLocation(tacoShader, "uvx_scale");
	static GLuint M_IT_ID = glGetUniformLocation(tacoShader, "M_IT");
	static GLuint texture_id = glGetUniformLocation(tacoShader, "texture");
	static GLuint LightID = glGetUniformLocation(tacoShader, "LightPosition_worldspace");
	static Mesh m = gen_taco();

	float uv_x_scale = (float)((RADIUS*PI) + 2.0f);
	glUseProgram(tacoShader);

	mat4 taco_basis = basis(taco_data.x_axis, taco_data.fold_center);
	mat4 tro = glm::translate(glm::mat4(), glm::vec3(-1 - RADIUS*PI / 2.0f, -1 / 2.0, 0.f));
	mat4 r = glm::rotate(glm::mat4(), 0.0f, vec3(0, 0, 1));
	mat4 s = glm::scale(mat4(), vec3(3.0, 3.0, -1.0));
	mat4 ModelMatrix = taco_basis*s*tro;
	mat4 MVP = taco_data.camera.projection* taco_data.camera.view* ModelMatrix;
	mat4 M_IT_MATRIX = inverse(transpose(ModelMatrix));

	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
	glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &taco_data.camera.view[0][0]);

	glUniformMatrix4fv(M_IT_ID, 1, GL_FALSE, &M_IT_MATRIX[0][0]);

	glUniform1f(uvx_scale_ID, uv_x_scale);

	glm::vec3 lightPos = glm::vec3(0.5, 0.5, 1);
	glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_back);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float color[] = { 0.0f, 0.3f, 0.3f, 0.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex_front);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glDisable(GL_DEPTH_TEST);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
#if 1
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_FRONT);
	glUniform1i(texture_id, 0);
	glUniform1f(normal_mul_ID, -1.0f);
	mesh_draw(&m);
#endif
#if 1
	glUniform1f(normal_mul_ID, 1.0f);
	glCullFace(GL_BACK);
	glUniform1i(texture_id, 1);
	mesh_draw(&m);
#endif
}

