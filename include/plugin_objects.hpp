#pragma once

#include <simulator_objects.hpp>

#include <GL/glew.h>

namespace plugin_objects
{
	extern int previous_depth_texture;
	extern int current_depth_texture;

	extern int cloud_map_textures[CLOUD_LAYER_COUNT];

	extern int base_noise_texture;
	extern int detail_noise_texture;

	extern int blue_noise_texture;

	extern int previous_rendering_texture;
	extern int current_rendering_texture;

	extern GLuint framebuffer;
	extern GLuint vertex_array;

	void initialize();
	void update();
};