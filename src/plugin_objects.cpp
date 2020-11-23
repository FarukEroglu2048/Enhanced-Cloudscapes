#include <plugin_objects.hpp>

#include <opengl_helpers.hpp>

#include <XPLMGraphics.h>

namespace plugin_objects
{
	int previous_depth_texture;
	int current_depth_texture;

	int cloud_map_textures[CLOUD_LAYER_COUNT];

	int base_noise_texture;
	int detail_noise_texture;

	int blue_noise_texture;

	int previous_rendering_texture;
	int current_rendering_texture;

	GLuint framebuffer;
	GLuint vertex_array;

	void initialize()
	{
		previous_depth_texture = create_fullscreen_texture();
		current_depth_texture = create_fullscreen_texture();

		cloud_map_textures[0] = load_png_texture("Resources/plugins/Enhanced Cloudscapes/textures/cloud_map_1.png", false);
		cloud_map_textures[1] = load_png_texture("Resources/plugins/Enhanced Cloudscapes/textures/cloud_map_2.png", false);
		cloud_map_textures[2] = load_png_texture("Resources/plugins/Enhanced Cloudscapes/textures/cloud_map_3.png", false);

		base_noise_texture = load_png_texture("Resources/plugins/Enhanced Cloudscapes/textures/base_noise.png", true);
		detail_noise_texture = load_png_texture("Resources/plugins/Enhanced Cloudscapes/textures/detail_noise.png", true);

		blue_noise_texture = load_png_texture("Resources/plugins/Enhanced Cloudscapes/textures/blue_noise.png", false);

		previous_rendering_texture = create_fullscreen_texture();
		current_rendering_texture = create_fullscreen_texture();

		GLint previous_framebuffer;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previous_framebuffer);

		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);

		glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, current_rendering_texture, 0);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, previous_framebuffer);

		glGenVertexArrays(1, &vertex_array);
		glBindVertexArray(vertex_array);

		GLuint vertex_buffer;
		glGenBuffers(1, &vertex_buffer);

		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

		GLfloat quad_vertices[] =
		{
			-1.0, -1.0,
			-1.0, 1.0,
			1.0, -1.0,

			1.0, -1.0,
			-1.0, 1.0,
			1.0, 1.0
		};

		glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, EMPTY_OBJECT);

		glBindVertexArray(EMPTY_OBJECT);
	}

	void update()
	{
		XPLMSetGraphicsState(0, 1, 0, 0, 0, 0, 0);
		glActiveTexture(GL_TEXTURE0);

		if ((simulator_objects::current_viewport.z != simulator_objects::previous_viewport.z) || (simulator_objects::current_viewport.w != simulator_objects::previous_viewport.w))
		{
			GLenum depth_format;

			if (simulator_objects::reverse_z == 0) depth_format = GL_DEPTH_COMPONENT24;
			else depth_format = GL_DEPTH_COMPONENT32F;

			glBindTexture(GL_TEXTURE_2D, previous_depth_texture);

			glTexImage2D(GL_TEXTURE_2D, 0, depth_format, simulator_objects::current_viewport.z, simulator_objects::current_viewport.w, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
			glCopyImageSubData(current_depth_texture, GL_TEXTURE_2D, 0, 0, 0, 0, previous_depth_texture, GL_TEXTURE_2D, 0, 0, 0, 0, simulator_objects::current_viewport.z, simulator_objects::current_viewport.w, 1);

			glBindTexture(GL_TEXTURE_2D, current_depth_texture);
			glCopyTexImage2D(GL_TEXTURE_2D, 0, depth_format, simulator_objects::current_viewport.x, simulator_objects::current_viewport.y, simulator_objects::current_viewport.z, simulator_objects::current_viewport.w, 0);
		}
		else
		{
			glCopyImageSubData(current_depth_texture, GL_TEXTURE_2D, 0, 0, 0, 0, previous_depth_texture, GL_TEXTURE_2D, 0, 0, 0, 0, simulator_objects::current_viewport.z, simulator_objects::current_viewport.w, 1);

			glBindTexture(GL_TEXTURE_2D, current_depth_texture);
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, simulator_objects::current_viewport.x, simulator_objects::current_viewport.y, simulator_objects::current_viewport.z, simulator_objects::current_viewport.w);
		}

		if ((simulator_objects::current_rendering_resolution.x != simulator_objects::previous_rendering_resolution.x) || (simulator_objects::current_rendering_resolution.y != simulator_objects::previous_rendering_resolution.y))
		{
			glBindTexture(GL_TEXTURE_2D, previous_rendering_texture);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, simulator_objects::current_rendering_resolution.x, simulator_objects::current_rendering_resolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			glCopyImageSubData(current_rendering_texture, GL_TEXTURE_2D, 0, 0, 0, 0, previous_rendering_texture, GL_TEXTURE_2D, 0, 0, 0, 0, simulator_objects::current_rendering_resolution.x, simulator_objects::current_rendering_resolution.y, 1);

			glBindTexture(GL_TEXTURE_2D, current_rendering_texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, simulator_objects::current_rendering_resolution.x, simulator_objects::current_rendering_resolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		}
		else glCopyImageSubData(current_rendering_texture, GL_TEXTURE_2D, 0, 0, 0, 0, previous_rendering_texture, GL_TEXTURE_2D, 0, 0, 0, 0, simulator_objects::current_rendering_resolution.x, simulator_objects::current_rendering_resolution.y, 1);

		XPLMBindTexture2d(EMPTY_OBJECT, 0);
	}
}