#include <plugin_objects.hpp>

#include <opengl_helpers.hpp>

#include <XPLMGraphics.h>

namespace plugin_objects
{
	int depth_texture;

	int cloud_map_textures[CLOUD_LAYER_COUNT];

	int base_noise_texture;
	int detail_noise_texture;

	int blue_noise_texture;

	int rendering_texture;

	GLuint framebuffer;
	GLuint vertex_array;

	void initialize()
	{
		depth_texture = create_fullscreen_texture();

		cloud_map_textures[0] = load_png_texture("Resources/plugins/Enhanced Cloudscapes/textures/cloud_map_1.png", false);
		cloud_map_textures[1] = load_png_texture("Resources/plugins/Enhanced Cloudscapes/textures/cloud_map_2.png", false);
		cloud_map_textures[2] = load_png_texture("Resources/plugins/Enhanced Cloudscapes/textures/cloud_map_3.png", false);

		base_noise_texture = load_png_texture("Resources/plugins/Enhanced Cloudscapes/textures/base_noise.png", true);
		detail_noise_texture = load_png_texture("Resources/plugins/Enhanced Cloudscapes/textures/detail_noise.png", true);

		blue_noise_texture = load_png_texture("Resources/plugins/Enhanced Cloudscapes/textures/blue_noise.png", false);

		rendering_texture = create_fullscreen_texture();

		GLint previous_framebuffer;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previous_framebuffer);

		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);

		glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, rendering_texture, 0);

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
		if ((simulator_objects::current_viewport.z != simulator_objects::previous_viewport.z) || (simulator_objects::current_viewport.w != simulator_objects::previous_viewport.w))
		{
			XPLMSetGraphicsState(0, 2, 0, 0, 0, 0, 0);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depth_texture);

			GLenum depth_format;

			if (simulator_objects::reverse_z == 0) depth_format = GL_DEPTH_COMPONENT24;
			else depth_format = GL_DEPTH_COMPONENT32F;

			glCopyTexImage2D(GL_TEXTURE_2D, 0, depth_format, simulator_objects::current_viewport.x, simulator_objects::current_viewport.y, simulator_objects::current_viewport.z, simulator_objects::current_viewport.w, 0);
			
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, rendering_texture);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, simulator_objects::current_viewport.z, simulator_objects::current_viewport.w, 0, GL_RGBA, GL_FLOAT, nullptr);	

			XPLMBindTexture2d(EMPTY_OBJECT, 0);
			XPLMBindTexture2d(EMPTY_OBJECT, 1);
		}
		else
		{
			XPLMSetGraphicsState(0, 1, 0, 0, 0, 0, 0);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depth_texture);

			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, simulator_objects::current_viewport.x, simulator_objects::current_viewport.y, simulator_objects::current_viewport.z, simulator_objects::current_viewport.w);

			XPLMBindTexture2d(EMPTY_OBJECT, 0);
		}
	}
}