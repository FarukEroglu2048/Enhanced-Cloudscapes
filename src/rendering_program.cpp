#include <rendering_program.hpp>

#include <opengl_helpers.hpp>

#include <simulator_objects.hpp>
#include <plugin_objects.hpp>

#include <XPLMGraphics.h>

#include <glm/gtc/type_ptr.hpp>

namespace rendering_program
{
	GLint reference;

	GLint near_clip_z;
	GLint far_clip_z;

	GLint previous_mvp_matrix;
	GLint current_mvp_matrix;

	GLint inverse_modelview_matrix;
	GLint inverse_projection_matrix;

	GLint skip_fragments;
	GLint frame_index;

	GLint sample_step_count;
	GLint sun_step_count;

	GLint maximum_sample_step_size;
	GLint maximum_sun_step_size;

	GLint use_blue_noise_dithering;

	GLint cloud_map_scale;

	GLint base_noise_scale;
	GLint detail_noise_scale;

	GLint blue_noise_scale;

	GLint cloud_types;

	GLint cloud_bases;
	GLint cloud_tops;

	GLint cloud_coverages;
	GLint cloud_densities;

	GLint base_noise_ratios;
	GLint detail_noise_ratios;

	GLint wind_offsets;

	GLint base_anvil;
	GLint top_anvil;

	GLint fade_start_distance;
	GLint fade_end_distance;

	GLint light_attenuation;

	GLint sun_direction;

	GLint sun_tint;
	GLint sun_gain;

	GLint ambient_tint;
	GLint ambient_gain;

	GLint mie_scattering;

	GLint atmosphere_bottom_tint;
	GLint atmosphere_top_tint;

	GLint atmospheric_blending;

	void initialize()
	{
		GLuint vertex_shader = load_shader("Resources/plugins/Enhanced Cloudscapes/shaders/rendering/vertex_shader.glsl", GL_VERTEX_SHADER);
		GLuint fragment_shader = load_shader("Resources/plugins/Enhanced Cloudscapes/shaders/rendering/fragment_shader.glsl", GL_FRAGMENT_SHADER);

		reference = create_program(vertex_shader, fragment_shader);
		glUseProgram(reference);

		GLint previous_depth_texture = glGetUniformLocation(reference, "previous_depth_texture");
		GLint current_depth_texture = glGetUniformLocation(reference, "current_depth_texture");

		GLint cloud_map_textures = glGetUniformLocation(reference, "cloud_map_textures");

		GLint base_noise_texture = glGetUniformLocation(reference, "base_noise_texture");
		GLint detail_noise_texture = glGetUniformLocation(reference, "detail_noise_texture");

		GLint blue_noise_texture = glGetUniformLocation(reference, "blue_noise_texture");

		GLint previous_rendering_texture = glGetUniformLocation(reference, "previous_rendering_texture");

		glUniform1i(previous_depth_texture, 0);
		glUniform1i(current_depth_texture, 1);

		GLint texture_indices[CLOUD_LAYER_COUNT] = {2, 3, 4};
		glUniform1iv(cloud_map_textures, CLOUD_LAYER_COUNT, texture_indices);

		glUniform1i(base_noise_texture, 5);
		glUniform1i(detail_noise_texture, 6);

		glUniform1i(blue_noise_texture, 7);

		glUniform1i(previous_rendering_texture, 8);

		near_clip_z = glGetUniformLocation(reference, "near_clip_z");
		far_clip_z = glGetUniformLocation(reference, "far_clip_z");

		previous_mvp_matrix = glGetUniformLocation(reference, "previous_mvp_matrix");
		current_mvp_matrix = glGetUniformLocation(reference, "current_mvp_matrix");

		inverse_modelview_matrix = glGetUniformLocation(reference, "inverse_modelview_matrix");
		inverse_projection_matrix = glGetUniformLocation(reference, "inverse_projection_matrix");

		skip_fragments = glGetUniformLocation(reference, "skip_fragments");
		frame_index = glGetUniformLocation(reference, "frame_index");

		sample_step_count = glGetUniformLocation(reference, "sample_step_count");
		sun_step_count = glGetUniformLocation(reference, "sun_step_count");

		maximum_sample_step_size = glGetUniformLocation(reference, "maximum_sample_step_size");
		maximum_sun_step_size = glGetUniformLocation(reference, "maximum_sun_step_size");

		use_blue_noise_dithering = glGetUniformLocation(reference, "use_blue_noise_dithering");

		cloud_map_scale = glGetUniformLocation(reference, "cloud_map_scale");

		base_noise_scale = glGetUniformLocation(reference, "base_noise_scale");
		detail_noise_scale = glGetUniformLocation(reference, "detail_noise_scale");

		blue_noise_scale = glGetUniformLocation(reference, "blue_noise_scale");

		cloud_types = glGetUniformLocation(reference, "cloud_types");

		cloud_bases = glGetUniformLocation(reference, "cloud_bases");
		cloud_tops = glGetUniformLocation(reference, "cloud_tops");

		cloud_coverages = glGetUniformLocation(reference, "cloud_coverages");
		cloud_densities = glGetUniformLocation(reference, "cloud_densities");

		base_noise_ratios = glGetUniformLocation(reference, "base_noise_ratios");
		detail_noise_ratios = glGetUniformLocation(reference, "detail_noise_ratios");

		wind_offsets = glGetUniformLocation(reference, "wind_offsets");

		base_anvil = glGetUniformLocation(reference, "base_anvil");
		top_anvil = glGetUniformLocation(reference, "top_anvil");

		fade_start_distance = glGetUniformLocation(reference, "fade_start_distance");
		fade_end_distance = glGetUniformLocation(reference, "fade_end_distance");

		light_attenuation = glGetUniformLocation(reference, "light_attenuation");

		sun_direction = glGetUniformLocation(reference, "sun_direction");

		sun_tint = glGetUniformLocation(reference, "sun_tint");
		sun_gain = glGetUniformLocation(reference, "sun_gain");

		ambient_tint = glGetUniformLocation(reference, "ambient_tint");
		ambient_gain = glGetUniformLocation(reference, "ambient_gain");

		mie_scattering = glGetUniformLocation(reference, "mie_scattering");

		atmosphere_bottom_tint = glGetUniformLocation(reference, "atmosphere_bottom_tint");
		atmosphere_top_tint = glGetUniformLocation(reference, "atmosphere_top_tint");

		atmospheric_blending = glGetUniformLocation(reference, "atmospheric_blending");

		glUseProgram(EMPTY_OBJECT);
	}

	void call()
	{
		XPLMSetGraphicsState(0, 9, 0, 0, 0, 0, 0);

		GLint previous_framebuffer;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previous_framebuffer);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, plugin_objects::framebuffer);
		glViewport(0, 0, simulator_objects::current_rendering_resolution.x, simulator_objects::current_rendering_resolution.y);

		glBindVertexArray(plugin_objects::vertex_array);

		glUseProgram(reference);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, plugin_objects::previous_depth_texture);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, plugin_objects::current_depth_texture);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, plugin_objects::cloud_map_textures[0]);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, plugin_objects::cloud_map_textures[1]);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, plugin_objects::cloud_map_textures[2]);

		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_3D, plugin_objects::base_noise_texture);

		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_3D, plugin_objects::detail_noise_texture);

		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, plugin_objects::blue_noise_texture);

		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, plugin_objects::previous_rendering_texture);

		glUniform1f(near_clip_z, simulator_objects::near_clip_z);
		glUniform1f(far_clip_z, simulator_objects::far_clip_z);

		glUniformMatrix4fv(previous_mvp_matrix, 1, GL_FALSE, glm::value_ptr(glm::mat4(simulator_objects::previous_mvp_matrix)));
		glUniformMatrix4fv(current_mvp_matrix, 1, GL_FALSE, glm::value_ptr(glm::mat4(simulator_objects::current_mvp_matrix)));

		glUniformMatrix4fv(inverse_modelview_matrix, 1, GL_FALSE, glm::value_ptr(glm::mat4(simulator_objects::inverse_modelview_matrix)));
		glUniformMatrix4fv(inverse_projection_matrix, 1, GL_FALSE, glm::value_ptr(glm::mat4(simulator_objects::inverse_projection_matrix)));

		glUniform1i(skip_fragments, simulator_objects::skip_fragments);
		glUniform1i(frame_index, simulator_objects::frame_index);

		glUniform1i(sample_step_count, simulator_objects::sample_step_count);
		glUniform1i(sun_step_count, simulator_objects::sun_step_count);

		glUniform1f(maximum_sample_step_size, simulator_objects::maximum_sample_step_size);
		glUniform1f(maximum_sun_step_size, simulator_objects::maximum_sun_step_size);

		glUniform1i(use_blue_noise_dithering, simulator_objects::use_blue_noise_dithering);

		glUniform1f(cloud_map_scale, simulator_objects::cloud_map_scale);

		glUniform1f(base_noise_scale, simulator_objects::base_noise_scale);
		glUniform1f(detail_noise_scale, simulator_objects::detail_noise_scale);

		glUniform1f(blue_noise_scale, simulator_objects::blue_noise_scale);

		glUniform1iv(cloud_types, CLOUD_LAYER_COUNT, simulator_objects::cloud_types);

		glUniform1fv(cloud_bases, CLOUD_LAYER_COUNT, simulator_objects::cloud_bases);
		glUniform1fv(cloud_tops, CLOUD_LAYER_COUNT, simulator_objects::cloud_tops);

		glUniform1fv(cloud_coverages, CLOUD_TYPE_COUNT, simulator_objects::cloud_coverages);
		glUniform1fv(cloud_densities, CLOUD_TYPE_COUNT, simulator_objects::cloud_densities);

		glUniform3fv(base_noise_ratios, CLOUD_TYPE_COUNT, reinterpret_cast<GLfloat*>(simulator_objects::base_noise_ratios));
		glUniform3fv(detail_noise_ratios, CLOUD_TYPE_COUNT, reinterpret_cast<GLfloat*>(simulator_objects::detail_noise_ratios));

		glUniform3fv(wind_offsets, CLOUD_LAYER_COUNT, reinterpret_cast<GLfloat*>(simulator_objects::wind_offsets));

		glUniform1f(base_anvil, simulator_objects::base_anvil);
		glUniform1f(top_anvil, simulator_objects::top_anvil);

		glUniform1f(fade_start_distance, simulator_objects::fade_start_distance);
		glUniform1f(fade_end_distance, simulator_objects::fade_end_distance);

		glUniform1f(light_attenuation, simulator_objects::light_attenuation);

		glUniform3fv(sun_direction, 1, glm::value_ptr(simulator_objects::sun_direction));

		glUniform3fv(sun_tint, 1, glm::value_ptr(simulator_objects::sun_tint));
		glUniform1f(sun_gain, simulator_objects::sun_gain);

		glUniform3fv(ambient_tint, 1, glm::value_ptr(simulator_objects::ambient_tint));
		glUniform1f(ambient_gain, simulator_objects::ambient_gain);

		glUniform1f(mie_scattering, simulator_objects::mie_scattering);

		glUniform3fv(atmosphere_bottom_tint, 1, glm::value_ptr(simulator_objects::atmosphere_bottom_tint));
		glUniform3fv(atmosphere_top_tint, 1, glm::value_ptr(simulator_objects::atmosphere_top_tint));

		glUniform1f(atmospheric_blending, simulator_objects::atmospheric_blending);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		XPLMBindTexture2d(EMPTY_OBJECT, 0);
		XPLMBindTexture2d(EMPTY_OBJECT, 1);

		XPLMBindTexture2d(EMPTY_OBJECT, 2);
		XPLMBindTexture2d(EMPTY_OBJECT, 3);
		XPLMBindTexture2d(EMPTY_OBJECT, 4);

		XPLMBindTexture2d(EMPTY_OBJECT, 5);
		XPLMBindTexture2d(EMPTY_OBJECT, 6);

		XPLMBindTexture2d(EMPTY_OBJECT, 7);

		XPLMBindTexture2d(EMPTY_OBJECT, 8);

		glUseProgram(EMPTY_OBJECT);

		glBindVertexArray(EMPTY_OBJECT);

		glViewport(simulator_objects::current_viewport.x, simulator_objects::current_viewport.y, simulator_objects::current_viewport.z, simulator_objects::current_viewport.w);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, previous_framebuffer);
	}
}