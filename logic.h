#pragma once


#define assert(x)if(!(x))__debugbreak();


vec2 get_pos(Entity *e) {
	return vec2(e->transform[3][0], e->transform[3][1]);
}
void set_pos(Entity *e, vec2 p) {
	e->transform[3][0] = p.x;
	e->transform[3][1] = p.y;
}


void add_control_points(DA_vec2 *points, Entity *curve) {
	static Mesh quad = gen_quad(vec2());
	for (int i = 0; i < points->length; i++) {
		Entity e = {};
		e.type = type_circle;
		e.transform = scale(translate(mat4(), vec3((*points)[i], 0.0)), vec3(0.05, 0.05, 1.0));
		e.color = vec3(1.0);
		e.affected_by_physics = false;
		e.mesh = &quad;
		e.layers = mouse_layer;
		e.bezier_point_data.index = i;
		e.bezier_point_data.points = points;
		e.bezier_point_data.curve = curve; // long term probably get some id here @EntityManager
		entities[num_entities++] = e;
	}
}


void init() {
	static Mesh quad = gen_quad(vec2());
	FoldPcs e;
	e.fold_bitmask = 0;
	e.mesh = gen_quad();
	folds_pcs[num_fold_pcs++] = e;


	{
		Entity e = {};
		e.type = type_circle;
		e.transform = scale(translate(mat4(), vec3(-0.2, -0.2, 0.0)), vec3(0.25));
		e.velocity = vec2(-0.0005, -0.0005);
		e.color = vec3(0.5, 0.8, 0.5);
		e.affected_by_physics = true;
		e.mesh = &quad;
		e.layers = gameplay_layer;
		entities[num_entities++] = e;
	}

	{
#if 0
		Entity e = {};
		e.type = type_circle;
		e.transform = scale(translate(mat4(), vec3(0.0, -0.2, 0.0)), vec3(0.25, 0.25, 1.0));
		e.velocity = vec2(-0.0005, 0.0005);
		e.color = vec3(1.0);
		e.affected_by_physics = true;
		e.mesh = &m;
		entities[num_entities++] = e;
#endif
	}
	{
		Entity floor = {};
		floor.layers = gameplay_layer;
		floor.type = type_wall;
		floor.transform = basis(vec2(1.0, 0.0), vec2(vec2(0.0, -0.5)));
		floor.affected_by_physics = false;
		floor.mesh = &quad;
		entities[num_entities++] = floor;

		Entity roof = {};
		roof.layers = gameplay_layer;
		roof.type = type_wall;
		roof.transform = basis(vec2(-1.0, 0.0), vec2(0.0, 0.5));
		roof.affected_by_physics = false;
		roof.mesh = &quad;
		entities[num_entities++] = roof;

		Entity left_wall = {};
		left_wall.layers = gameplay_layer;
		left_wall.type = type_wall;
		left_wall.transform = basis(vec2(0.0, -1.0), vec2(-0.5, 0.0));
		left_wall.affected_by_physics = false;
		left_wall.mesh = &quad;
		entities[num_entities++] = left_wall;

		Entity right_wall = {};
		right_wall.layers = gameplay_layer;
		right_wall.type = type_wall;
		right_wall.transform = basis(vec2(0.0, 1.0), vec2(0.5, 0.0));
		right_wall.affected_by_physics = false;
		right_wall.mesh = &quad;
		entities[num_entities++] = right_wall;
	}
	{
#if 1
		static DA_vec2 points = DA_vec2::make(default_allocator, 16);
		points.length = 0.0;
		points.add(vec2(0.0, 0.0));
		points.add(vec2(0.5, 0.0));
		points.add(vec2(0.5, 0.5));
		points.add(vec2(0.0, 0.5));
		points.add(vec2(-0.5, 0.5));
		points.add(vec2(-0.5, 0.0));

		static Mesh beiz = gen_beizpath(points, 0.01f);

		static Entity curve = {};
		curve.type = type_curve;
		curve.transform = mat4();
		curve.affected_by_physics = false;
		curve.mesh = &beiz;
		curve.color = vec3(1.0, 0.0, 1.0);
		curve.layers = (CollisionLayers)(gameplay_layer | mouse_layer);
		curve.bezier_point_data.curve = &curve;
		curve.bezier_point_data.index = -1;
		curve.bezier_point_data.points = &points;

		entities[num_entities++] = curve;
		add_control_points(&points, &curve);
#endif
	}
}


vec2 extreme_point_in_direction(Entity *e, vec2 dir) {
	// note transpose here is not some cheap sometimes correct inverse
	// it's actually what we want :) 
	// think about model matrix equal to 
	// 
	// |1 1|
	// |0 1| 
	//
	// if we're looking for max in [1,0], we should look in [1,1] dir

	vec2 object_space_dir = normalize(vec2(vec4(dir, 0, 0)*e->transform));
	vec2 r;
	switch (e->type) {
	case type_circle:
		r = object_space_dir*0.5f;
		break;
	case type_square:
		r.x = object_space_dir.x > 0 ? 0.5 : -0.5;
		r.y = object_space_dir.y > 0 ? 0.5 : -0.5;
		break;
	case type_wall:
		float inf = 5;
		r.x = object_space_dir.x >= 0 ? inf : -inf;
		r.y = object_space_dir.y >= 0 ? 0 : -inf;
		break;
	}
	vec2 ret = vec2(e->transform*vec4(r, 0, 1.0)); //back into world space
	return ret;
}

vec2 extreme_point_in_direction(Collider *c, vec2 dir) {
	if (c->type == collider_entity) {
		Entity *e = c->entity;
		if (e->type == type_square) {
			vec2 verts[4];
			verts[0] = vec2(0.5, 0.5);
			verts[1] = vec2(-0.5, -0.5);
			verts[2] = vec2(-0.5, 0.5);
			verts[3] = vec2(0.5, -0.5);

			for (int i = 0; i < 4; i++) {
				verts[i] = vec2(e->transform*vec4(verts[i], 0, 1));
			}
			vec2 correct = extreme_point_in_direction(e, dir);

			float longest = -FLT_MAX;
			vec2 res = vec2(-1, -1);
			for (int i = 0; i < 4; i++) {
				float c = dot(dir, verts[i]);
				if (c > longest) {
					longest = c;
					res = verts[i];
				}
			}

			return res;
		}
		return extreme_point_in_direction(c->entity, dir);
	} else if (c->type == collider_line_quad) {
		vec2 *verts = (vec2 *)c->p;
		float longest = -FLT_MAX;
		vec2 res = vec2(-1, -1);
		for (int i = 0; i < 4; i++) {
			float c = dot(dir, verts[i]);
			if (c > longest) {
				longest = c;
				res = verts[i];
			}
		}
		return res;
	} else if (c->type == collider_line) {
		vec2 *verts = (vec2 *)c->p;
		float longest = -FLT_MAX;
		vec2 res = vec2(-1, -1);
		for (int i = 0; i < 2; i++) {
			float c = dot(dir, verts[i]);
			if (c > longest) {
				longest = c;
				res = verts[i];
			}
		}
		return res;
	} else if (c->type == collider_convex_mesh) {
		// if we sort the mesh by angle we could binary search. but meeehh.
		Mesh *m = c->mesh;
		mat4 transform = *c->transform;
		vec3 *verts = m->verts.start;
		float longest = -FLT_MAX;
		vec2 res = vec2(-1, -1);
		for (int i = 0; i < m->verts.length; i++) {
			vec2 v = vec2(transform*vec4(verts[i], 1));
			float c = dot(dir, v);
			if (c > longest) {
				longest = c;
				res = vec2(v);
			}
		}
		return res;
	} else if (c->type == collider_point) {
		return vec2(c->p[0].x, c->p[0].y);
	}

}
vec2 extreme_point_in_direction_epa(Collider *c, vec2 dir) {
	if (c->type == collider_line_quad) {
		vec2 *verts = (vec2 *)c->p;
		vec2 ca = normalize(verts[0] - verts[2]);
		float longest = -FLT_MAX;
		vec2 res = vec2(-1, -1);
		for (int i = 0; i < 4; i++) {
			float c = dot(dir, verts[i]);
			if (c > longest) {
				longest = c;
				res = verts[i] + ((i < 2) ? ca : -ca);
			}
		}
		return res;
	} else {
		return extreme_point_in_direction(c, dir);
	}
}

vec2 support(Collider *a, Collider *b, vec2 dir) {
	return extreme_point_in_direction(a, dir) - extreme_point_in_direction(b, -dir);
}

vec2 support_epa(Collider *a, Collider *b, vec2 dir) {
	return extreme_point_in_direction_epa(a, dir) - extreme_point_in_direction_epa(b, -dir);
}


struct Simplex {
	vec2 difference_points[3];
	vec2 real_points[3];
	int len;
};

void do_simplex(vec2 *simplex, int *len, vec2 *dir) {
	switch (*len) {
#define SAME_DIR(e,bott) dot(e,bott) > 0
#define IF(e) if(SAME_DIR(e,ao))
	case 2:
	{
		vec3 b = vec3(simplex[0], 0);
		vec3 a = vec3(simplex[1], 0);
		vec3 ab = b - a;
		vec3 ao = -a;
		IF(ab) {
			*dir = vec2(cross(cross(ab, ao), ab));
		} else {
			*len = 1;
			simplex[0] = vec2(a);
			*dir = vec2(ao);
		}
		break;
	}
	case 3:
		vec3 a, b, c, ab, ac, ao;
		vec3 abc;
		a = vec3(simplex[2], 0);
		b = vec3(simplex[1], 0);
		c = vec3(simplex[0], 0);
		ao = -a;
		ab = b - a;
		ac = c - a;
		abc = cross(ab, ac);
		IF(cross(abc, ac)) {
			IF(ac) {
				simplex[0] = vec2(a);
				simplex[1] = vec2(c);
				*len = 2;
				*dir = vec2(cross(cross(ac, ao), ac));
			} else {
				IF(ab) {
					simplex[0] = vec2(a);
					simplex[1] = vec2(b);
					*len = 2;
					*dir = vec2(cross(cross(ab, ao), ab));
				} else {
					simplex[0] = vec2(a);
					*len = 1;
					*dir = vec2(ao);
				}
			}
		} else {
			IF(cross(ab, abc)) {
				IF(ab) {
					simplex[0] = vec2(a);
					simplex[1] = vec2(b);
					*len = 2;
					*dir = vec2(cross(cross(ab, ao), ab));
				} else {
					simplex[0] = vec2(a);
					*len = 1;
					*dir = vec2(ao);
				}
			} else {
				//it is enclosing!
				return;
			}
		}

		break;
	}
#undef SAME_DIR
#undef IF
}



bool is_colliding_sub(Collider *a, Collider *b, vec2 *pen, vec2 *normal) {
	static DA_vec2 polytope = DA_vec2::make(default_allocator, 16);
	polytope.length = 0;
	//using gjk (polytope is just a simplex here)
	polytope.add(support(a, b, vec2(0, 1)));
	vec2 dir = -polytope[0];
	for (;;) {
		polytope.add(support(a, b, dir));
		vec2 d = polytope[1] - polytope[0];
		if (dot(polytope[polytope.length - 1], dir) < 0) return false;
		do_simplex(polytope.start, &polytope.length, &dir);
		if (polytope.length == 3) {
			break;
		}
	}
	if (!pen)return true;

	//using epa to expand the simplex into a polytope
	{
		if (signed_area(polytope[0], polytope[1], vec2()) < 0) {
			std::swap(polytope[0], polytope[1]);
		}


		float old_shortest = FLT_MAX;
		for (;;) {
			float shortest = FLT_MAX;
			int index = -1;
			vec2 norm = vec2(-1, -1);
			// find edge with smallest distance to origo
			for (int i = 0; i < polytope.length; i++) {
				vec2 a = polytope[i];
				vec2 b = polytope[(i + 1) % polytope.length];
				vec2 ab = b - a;
				vec2 ao = -a;
				vec2 n = normalize(vec2(ab.y, -ab.x));
				float area = signed_area(a, b, vec2());
				float dist_origo = dot(n, ao);

				assert(dist_origo >= 0.0 || (ab.x == 0.0&&ab.y == 0.0));
				if (dist_origo < shortest) {
					shortest = dist_origo;
					index = i;
					norm = n;
				}
			}

			vec2 p = support_epa(a, b, -norm);

			if (fabs(old_shortest - shortest) < 0.001f) {
				*pen = norm*shortest;
				*normal = -norm;
				return true;
			}

			old_shortest = shortest;
			int len = polytope.length;
			// expand convex hull
			float a_prev = signed_area(p, polytope[polytope.length - 1], vec2());
			for (int i = 0; i < polytope.length; i++) {
				float a_next = signed_area(p, polytope[i], vec2());
				if (a_next > 0 && a_prev < 0) {
					polytope.insert(p, i);
					break;
				}
				a_prev = a_next;
			}
		}
	}
}



Collider *get_colliders(Entity *e, int *len) {
	if (e->type != type_curve) {
		Collider *c = (Collider *)malloc(sizeof(Collider));
		c->type = collider_entity;
		c->entity = e;
		*len = 1;
		return c;
	} else {
		*len = e->mesh->indices.length / 6;
		Collider *c1 = (Collider *)malloc(sizeof(Collider)* *len);
		for (int i = 0; i < e->mesh->indices.length; i += 6) {
			vec2 polygon[4];
			vec2 a = vec2(e->mesh->verts[e->mesh->indices[i]]);
			vec2 b = vec2(e->mesh->verts[e->mesh->indices[i + 1]]);
			vec2 c = vec2(e->mesh->verts[e->mesh->indices[i + 2]]);
			vec2 d = vec2(e->mesh->verts[e->mesh->indices[i + 4]]);
			c1[i / 6].p[0].x = a.x;
			c1[i / 6].p[0].y = a.y;
			c1[i / 6].p[1].x = b.x;
			c1[i / 6].p[1].y = b.y;
			c1[i / 6].p[2].x = c.x;
			c1[i / 6].p[2].y = c.y;
			c1[i / 6].p[3].x = d.x;
			c1[i / 6].p[3].y = d.y;
			c1[i / 6].type = collider_line_quad;
		}
		return c1;
	}
}

bool is_colliding(Entity *a, Entity *b, vec2 *pen, vec2 *normal) {
	if (!(a->layers & b->layers)) return false;
	bool swapped = false;
	if (!a->affected_by_physics && !b->affected_by_physics) return false;

	int len_a, len_b;
	Collider *ca = get_colliders(a, &len_a);
	Collider *cb = get_colliders(b, &len_b);
	float max = 0;
	vec2 pen_ret;
	vec2 normal_ret;
	bool ret = false;
	for (int i = 0; i < len_a; i++) {
		for (int j = 0; j < len_b; j++) {
			if (is_colliding_sub(&ca[i], &cb[j], pen, normal)) {
				ret = true;
				if (dot(*pen, *pen) > max) {
					max = dot(*pen, *pen);
					pen_ret = *pen;
					normal_ret = *normal;
				}
			}
		}
	}
	free(ca);
	free(cb);
	if (ret) {
		*pen = pen_ret;
		*normal = normal_ret;
		return true;
	} else {
		return false;
	}

}



vec2 gravity(Entity *e) {
	for (int i = 0; i < num_fold_pcs; i++) {
		Collider ca, cb;
		ca.type = collider_point;
		vec2 p = get_pos(e);
		ca.p[0] = { p.x,p.y + (p.x == p.y)*0.0001f };
		cb.type = collider_convex_mesh;
		cb.mesh = &folds_pcs[i].mesh;
		mat4 m = glm::scale(mat4(), vec3(-1, -1, 1.0f)) * glm::translate(mat4(), vec3(-0.5f, -0.5f, 0.0f));


		cb.transform = &m;
		vec2 pen_vec;
		if (is_colliding_sub(&ca, &cb, NULL, NULL)) {
			return vec2(vec4(0, -1, 0, 1) * getFoldPcsTransform_no_transform(i));
		}
	}
	return vec2(0, 1);
}


void physics() {
	for (int i = 0; i < num_entities; i++) {
		Entity *e = &entities[i];
		if (!e->affected_by_physics) continue;
		vec2 p = get_pos(e);
		p += e->velocity;
		set_pos(e, p);
		vec2 gravity_dir = gravity(e);
		entities[i].velocity += gravity_dir*0.0001f;
	}


	for (int i = 0; i < num_entities; i++) {

		for (int j = i + 1; j < num_entities; j++) {
			if (!entities[i].affected_by_physics && !entities[j].affected_by_physics)continue;
			vec2 pa = get_pos(&entities[i]);
			vec2 pb = get_pos(&entities[j]);
			vec2 rel_velocity = entities[i].velocity - entities[j].velocity;
			vec2 pen, normal;
			if (is_colliding(&entities[i], &entities[j], &pen, &normal)) {

				//if (dot(pa - pb, pen) < 0)pen = -pen; // don't like this ehh
				//printf("[%d] (%d,%d),colliding, pen = (%f %f) rel= (%f %f)\n", globals.frame, i, j, pen.x, pen.y, rel_velocity.x, rel_velocity.y);
				vec2 col_normal = normal;

				float t = dot(rel_velocity, col_normal);
				vec2 rel_vel_along_normal = t*col_normal;
				float p = 0.5;
				if (!entities[i].affected_by_physics) p = 0.0;
				if (!entities[j].affected_by_physics) p = 1.0;

				pa += pen*p;
				pb -= pen*(1 - p);

				set_pos(&entities[i], pa);
				set_pos(&entities[j], pb);

				if (t < 0 || 1) {
					entities[i].velocity -= 2 * p*rel_vel_along_normal*0.8f;
					entities[j].velocity += 2 * (1 - p)*rel_vel_along_normal*0.8f;
				}
			}
		}
	}
}




void update() {


	vec2 v = entities[0].velocity;

	float speed = 0.004;
	if (input[key_right].down) v.x = max(speed, v.x);
	if (input[key_left].down) v.x = min(-speed, v.x);
	if (input[key_up].down) v.y = max(speed, v.y);
	if (input[key_down].down) v.y = min(-speed, v.y);

	entities[0].velocity = v;

	static Entity *clicked_entity = 0;
	Entity *curve = NULL;
	bool update_beiz = false;
	int collider = 0;
	if (input[mouse_left].press) {
		vec2 start = input[mouse_left].mouse_pos_on_press;
		vec2 curr = globals.mouse_pos;
		Collider c;
		c.type = collider_point;
		c.p[0].x = curr.x;
		c.p[0].y = curr.y;


		for (int i = 0; i < num_entities; i++) {
			int len;
			if (!(entities[i].layers & mouse_layer))continue;
			Collider *a = get_colliders(&entities[i], &len);
			for (int j = 0; j < len; j++) {
				if (is_colliding_sub(&c, &a[j], 0, 0)) {
					if (entities[i].type == type_circle) {
						clicked_entity = &entities[i];
					} else if (entities[i].type == type_curve) {
						collider = j;
						curve = entities[i].bezier_point_data.curve;
						update_beiz = true;
					}
				}
			}
			free(a);
		}
	}
	if (update_beiz && curve && !clicked_entity) {
		float s = curve->mesh->uvs[curve->mesh->indices[collider * 6]].y;
		float e = curve->mesh->uvs[curve->mesh->indices[collider * 6 + 4]].y;

		split(curve->bezier_point_data.points, (s + e) / 2);
		*curve->mesh = gen_beizpath(*curve->bezier_point_data.points, 0.01f);

		clicked_entity = NULL;
		for (int i = num_entities; i >= 0; i--) {
			if (entities[i].type == type_circle && entities[i].bezier_point_data.curve == curve) {
				entities[i] = entities[--num_entities];
			}
		}
		add_control_points(curve->bezier_point_data.points,curve);
	}

	if (clicked_entity) {
		Entity *curve = clicked_entity->bezier_point_data.curve;
		DA_vec2 *points = clicked_entity->bezier_point_data.points;
		int idx = clicked_entity->bezier_point_data.index;
		(*points)[idx] = globals.mouse_pos;
		mesh_destroy(curve->mesh);
		*curve->mesh = gen_beizpath(*points, 0.01);
		set_pos(clicked_entity, globals.mouse_pos);
	}

	if (input[mouse_left].release) {
		clicked_entity = 0;
	}

	physics();
	vec2 p = get_pos(&entities[0]);

	mat4 scew(
		1.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	entities[0].transform = scale(translate(mat4(), vec3(p.x, p.y, 0.0)), vec3(0.1f, 0.1f, 1.0f));
}
