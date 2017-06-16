#define PI ((float)acos(-1.0))

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <GL/glew.h>
#include <glfw3.h>
GLFWwindow* window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "DataStructures.h"
#include "maths.hpp"
#include "graphics.hpp"
#include "draw.hpp"



void update_animations() {
	for (int i = animations.length - 1; i >= 0; i--) {
		Animation anim = animations[i];
		double t = (globals.current_time - anim.start_time) / anim.animation_time;
		switch (anim.mode) {
		case animation_mode_linear:
		{
			if (t > 1.0) {
				if (anim.animating)*anim.animating = false;
				t = 1.0;
				animations.remove(i);
			}
			*anim.data = lerp(anim.start_value, anim.end_value, (float)t);
		} break;
		}
	}
}

void start_animation(float *to_anim, float animation_time, float to, AnimationMode mode, bool *animating, bool remove_dup = false) {
	Animation anim = {};
	anim.start_time = (float)globals.current_time;
	anim.animation_time = animation_time;
	anim.data = to_anim;

	anim.start_value = *to_anim;
	anim.end_value = to;

	anim.animating = animating;
	anim.mode = mode;
	if (animating)*animating = true;

	if (remove_dup) {
		for (int i = 0; i < animations.length; i++) {
			if (animations[i].data == to_anim) {
				animations[i] = anim;
				return;
			}
		}
	}
	animations.add(anim);
}
#include "logic.h"


int glfw_button_from_input(InputType type) {
	switch (type) {
	case mouse_left:  return GLFW_MOUSE_BUTTON_LEFT;
	case mouse_right: return GLFW_MOUSE_BUTTON_RIGHT;
	case key_left:    return GLFW_KEY_LEFT;
	case key_right:   return GLFW_KEY_RIGHT;
	case key_up:      return GLFW_KEY_UP;
	case key_down:    return GLFW_KEY_DOWN;
	default: assert(false);
	}
	return -1;
}

bool is_mouse_input(InputType type) {
	switch (type) {
	case mouse_left:
	case mouse_right:
		return true;
	case key_left:
	case key_right:
	case key_up:
	case key_down:
		return false;
	default: assert(false);
	}
	return -1;
}

void update_input() {

	globals.mouse_pos = (get_mousepos_in_ws(taco_data.camera.projection, taco_data.camera.view, 0)-vec2(0.5)) * -2.0f;
	for (int i = 0; i < input_end; i++) {
		InputType button = (InputType)i;
		input[button].press = false;
		input[button].release = false;
		bool down;
		if (is_mouse_input(button)) down = glfwGetMouseButton(window, glfw_button_from_input(button)) == GLFW_PRESS;
		else down = glfwGetKey(window, glfw_button_from_input(button)) == GLFW_PRESS;

		if (down) {
			input[button].press = !input[button].down;
			input[button].down = true;
			if (input[button].press) {
				input[button].mouse_pos_on_press = globals.mouse_pos;
			}
		} else {
			input[button].release = input[button].down;
			input[button].down = false;
		}
	}
}



int main(void) {
	// Initialise GLFW
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "Tutorial 09 - VBO Indexing", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	printOpenGLError();

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);



	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;

	glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	glEnable(GL_BLEND);
	GLuint texture_back;
	GLuint texture_front;
	MRT_Settings settings = {};
	static GLuint render_target_front = make_fbo(&texture_back, settings);
	static GLuint render_target_back = make_fbo(&texture_front, settings);


	float FoV = 80.0f;
	vec3 position = vec3(0.5, 0.5, 0.5);
	vec3 direction = vec3(0.0f, 0.0f, -1.0f);
	vec3 up = vec3(0.0f, 1.0f, 0.0f);
	taco_data.camera.projection = glm::perspective(FoV, 4.0f / 3.0f, 0.1f, 100.0f);
	//taco_data.camera.projection = glm::ortho(-0.5,0.5,-0.5,0.5);
	taco_data.camera.view = lookAt(position, position + direction, up);


	init();

	do {
		update_input();
		++globals.frame;
		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1sec ago
			// printf and reset
			printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}
		globals.delta_time = currentTime - globals.current_time;
		globals.current_time = currentTime;
		update_animations();
		update();

		vec2 p = get_mousepos_in_ws(taco_data.camera.projection, taco_data.camera.view, 0);

		static vec2 fold_center;
		static vec2 x_taco_space;

		//printf("(%f%f)\n", p.x, p.y);
		static bool mouse_down = false;
		static vec2 start_mouse_drag;
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && activated_trasforms == 1.0f) {
			if (!mouse_down) {
				start_mouse_drag = p;
				mouse_down = true;
			}
		} else if (mouse_down) {
			mouse_down = false;
			fold_at(x_taco_space, fold_center);
			draw_front_back(render_target_front, render_target_back, texture_size);
		}

		if (mouse_down && p != start_mouse_drag) {
			x_taco_space = start_mouse_drag - p;
			fold_center = p + x_taco_space / vec2(2.0);
			x_taco_space = normalize(x_taco_space);
		} else {
			x_taco_space = vec2(1, 0);
			fold_center = vec2(2.0, 0.0);
		}


		taco_data.x_axis = x_taco_space;
		taco_data.fold_center = fold_center;

		draw_front_back(render_target_front, render_target_back, texture_size);
		draw_taco(texture_front, texture_back);

		glfwSwapBuffers(window);
		glfwPollEvents();
		static bool fold_animating = false;
		static bool down_space = false;
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			if (!down_space && !fold_animating) {
				start_animation(&activated_trasforms, 2.0f, 1.0f - round(activated_trasforms), animation_mode_linear, &fold_animating);
				down_space = true;
				printf("new_anim\n");
			}
		} else down_space = false;
	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);


	glDeleteTextures(1, &texture_front);
	glDeleteTextures(1, &texture_back);

	return 0;
	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	return 0;
}

#define max(a,b) ((a)>(b)? (a):(b))
#include "L:\Mem.cpp"
