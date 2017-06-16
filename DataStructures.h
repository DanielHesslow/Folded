#pragma once

/*
	Here goes all of our data structures and global data
*/

enum DrawMode {
	draw_mode_wire_frame,
	draw_mode_solid,
	draw_mode_points,
};

#include "L:\Mem.h"

#define DA_TYPE vec3
#define DA_NAME DA_vec3
#include "L:\dynamicarray.h"

#define DA_TYPE vec2
#define DA_NAME DA_vec2
#include "L:\dynamicarray.h"


#define DA_TYPE uint16_t
#define DA_NAME DA_u16
#include "L:\dynamicarray.h"

DH_PlatformAllocator pa;
DH_Allocator *p_pa = (DH_Allocator *)&pa;
DH_GeneralPurposeAllocator gp(p_pa);
DH_Allocator *default_allocator = (DH_Allocator *)&gp;

struct Mesh {
	DA_vec3 verts;
	DA_vec3 normals;
	DA_vec2 uvs;
	DA_u16  indices;

	GLuint vao_id;
	GLuint vertex_buffer;
	GLuint normal_buffer;
	GLuint uv_buffer;
	GLuint index_buffer;

	static Mesh make() {
		Mesh m = {};
		m.verts = DA_vec3::make(default_allocator);
		m.normals = DA_vec3::make(default_allocator);
		m.uvs = DA_vec2::make(default_allocator);
		m.indices = DA_u16::make(default_allocator);
		return m;
	}
};

struct VertexInfo {
	vec3 vert;
	vec3 normal;
	vec2 uv;
};


uint32_t mem_hash(void *ptr, int len) {
	int ack = 0;
	for (int i = 0; i < len; i++) {
		ack = ack * 101 + ((char *)ptr)[i];
	}
	return ack;
}

#define HT_KEY   VertexInfo
#define HT_VALUE int
#define HT_NAME  IndexFromVertex
#define HT_HASH(x) mem_hash(&x,sizeof(VertexInfo))
#define HT_EQUAL(a,b) (a.vert == b.vert && a.normal == b.normal && a.uv == b.uv)

#include "L:\HashTable.h"

struct MeshBuilder {
	Mesh *mesh;
	IndexFromVertex map;
};

struct MRT_Settings {
	int samples;
	bool nearest;
	int size;
};

enum AnimationMode {
	animation_mode_linear,
};

struct Animation {
	float *data;
	bool  *animating;

	float start_value;
	float end_value;

	float start_time;
	float animation_time;
	AnimationMode mode;
};

struct FoldPcs {
	Mesh mesh;
	int fold_bitmask;
};

static FoldPcs folds_pcs[1024];
int num_fold_pcs = 0;
static mat4 fold_transforms[20];
float activated_trasforms = 0.0;
int num_folds = 0;

struct {
	double current_time;
	double delta_time;
	int frame;
	vec2 mouse_pos;
} globals;

#define DA_TYPE Animation
#define DA_NAME DA_Animation
#include "L:\dynamicarray.h"

DA_Animation animations = DA_Animation::make(default_allocator);

struct Camera {
	mat4 projection;
	mat4 view;
};

struct {
	vec2 x_axis;
	vec2 fold_center;
	Camera camera;
} taco_data;


const int texture_size = 1024;

#define UV_X_SCALE (RADIUS*PI + 2)


enum EntityType {
	type_player,
	type_circle,
	type_square,
	type_curve,
	type_wall,
};


enum CollisionLayers {
	mouse_layer=1,
	gameplay_layer,
};

struct Entity {
	mat4 transform;
	vec2 velocity;
	vec3 color;
	EntityType type;
	Mesh *mesh;
	bool affected_by_physics;
	union {
		struct {
			DA_vec2 *points;
			int index;
			Entity *curve;
		}bezier_point_data;
	};
	CollisionLayers layers;
};


Entity entities[1024];
int num_entities = 0;


enum ColliderType {
	collider_entity,
	collider_convex_mesh,
	collider_line_quad,
	collider_line,
	collider_point,
};


struct Collider {
	ColliderType type;
	union {
		struct {
			float x, y;
		}p[4];
		Entity *entity;
		struct {
			Mesh *mesh;
			mat4 *transform;
		};
	};
};


enum InputType {
	mouse_left,
	mouse_right,
	key_left,
	key_right,
	key_up,
	key_down,
	input_end
};

struct Input {
	bool down, press, release;

	// mouse stuff
	vec2 mouse_pos_on_press;

};

static Input input[input_end];


