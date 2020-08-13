#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <glm/mat4x4.hpp>

#define CLOUD_LAYER_COUNT 3
#define CLOUD_TYPE_COUNT 5

namespace simulator_objects
{
	extern int version;

	extern glm::ivec4 previous_viewport;
	extern glm::ivec4 current_viewport;

	extern glm::ivec2 previous_rendering_resolution;
	extern glm::ivec2 current_rendering_resolution;

	extern int skip_fragments;
	extern int frame_index;

	extern int reverse_z;

	extern float near_clip_z;
	extern float far_clip_z;

	extern glm::mat4 inverse_projection_matrix;
	extern glm::mat4 inverse_modelview_matrix;

	extern float cloud_map_scale;

	extern float base_noise_scale;
	extern float detail_noise_scale;

	extern float blue_noise_scale;

	extern int cloud_types[CLOUD_LAYER_COUNT];

	extern float cloud_bases[CLOUD_LAYER_COUNT];
	extern float cloud_tops[CLOUD_LAYER_COUNT];

	extern float cloud_coverages[CLOUD_TYPE_COUNT];
	extern float cloud_densities[CLOUD_TYPE_COUNT];

	extern glm::vec3 base_noise_ratios[CLOUD_TYPE_COUNT];
	extern glm::vec3 detail_noise_ratios[CLOUD_TYPE_COUNT];

	extern glm::vec3 wind_offsets[CLOUD_LAYER_COUNT];

	extern float fade_start_distance;
	extern float fade_end_distance;

	extern glm::vec3 sun_direction;

	extern glm::vec3 sun_tint;
	extern float sun_gain;

	extern glm::vec3 ambient_tint;
	extern float ambient_gain;

	extern float backward_mie_scattering;
	extern float forward_mie_scattering;

	extern glm::vec3 atmosphere_bottom_tint;
	extern glm::vec3 atmosphere_top_tint;

	extern float atmospheric_blending;

	void initialize();
	void update();
}