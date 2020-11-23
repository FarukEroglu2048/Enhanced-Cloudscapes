#include <opengl_helpers.hpp>

#include <XPLMUtilities.h>
#include <XPLMGraphics.h>

#include <png.h>

#include <fstream>

void read_stream_callback(png_structp png_struct, png_bytep output_data, png_size_t read_size)
{
	png_voidp stream_pointer = png_get_io_ptr(png_struct);

	std::ifstream& input_stream = *static_cast<std::ifstream*>(stream_pointer);
	input_stream.read(reinterpret_cast<char*>(output_data), read_size);
}

png_size_t get_bytes_per_pixel(png_byte color_type)
{
	switch (color_type)
	{
		case PNG_COLOR_TYPE_GRAY:
			return 1;
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			return 2;
		case PNG_COLOR_TYPE_RGB:
			return 3;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			return 4;
	}
}

GLenum get_texture_format(png_byte color_type)
{
	switch (color_type)
	{
		case PNG_COLOR_TYPE_GRAY:
			return GL_RED;
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			return GL_RG;
		case PNG_COLOR_TYPE_RGB:
			return GL_RGB;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			return GL_RGBA;
	}
}

png_size_t integer_square_root(png_size_t input_value)
{
	png_size_t square_root = 0;
	while ((square_root * square_root) < input_value) square_root++;

	return square_root;
}

int create_fullscreen_texture()
{
	int texture_reference;
	XPLMGenerateTextureNumbers(&texture_reference, 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_reference);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	XPLMBindTexture2d(EMPTY_OBJECT, 0);

	return texture_reference;
}

int create_texture(GLsizei texture_width, GLsizei texture_height, GLenum data_format, const void* texture_data)
{
	int texture_reference;
	XPLMGenerateTextureNumbers(&texture_reference, 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_reference);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, data_format, texture_width, texture_height, 0, data_format, GL_UNSIGNED_BYTE, texture_data);

	XPLMBindTexture2d(EMPTY_OBJECT, 0);

	return texture_reference;
}

int create_texture(GLsizei texture_width, GLsizei texture_height, GLsizei texture_depth, GLenum data_format, const void* texture_data)
{
	int texture_reference;
	XPLMGenerateTextureNumbers(&texture_reference, 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, texture_reference);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage3D(GL_TEXTURE_3D, 0, data_format, texture_width, texture_height, texture_depth, 0, data_format, GL_UNSIGNED_BYTE, texture_data);

	XPLMBindTexture2d(EMPTY_OBJECT, 0);

	return texture_reference;
}

int load_png_texture(const char* texture_path, bool add_depth)
{
	int output_texture = EMPTY_OBJECT;

	std::ifstream texture_file(texture_path, std::ifstream::binary);

	if (texture_file.fail() == false)
	{
		png_structp png_struct = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
		png_infop png_info = png_create_info_struct(png_struct);

		png_set_read_fn(png_struct, &texture_file, read_stream_callback);

		png_read_info(png_struct, png_info);

		png_set_scale_16(png_struct);
		png_set_expand(png_struct);

		png_byte color_type = png_get_color_type(png_struct, png_info);

		png_size_t bytes_per_pixel = get_bytes_per_pixel(color_type);

		png_uint_32 image_width = png_get_image_width(png_struct, png_info);
		png_uint_32 image_height = png_get_image_height(png_struct, png_info);

		png_bytep texture_data = new png_byte[image_height * image_width * bytes_per_pixel];
		png_bytepp texture_row_pointers = new png_bytep[image_height];

		for (png_size_t row_index = 0; row_index < image_height; row_index++) texture_row_pointers[row_index] = &texture_data[row_index * image_width * bytes_per_pixel];

		png_read_image(png_struct, texture_row_pointers);

		if (add_depth == true) output_texture = create_texture(image_width, integer_square_root(image_height), integer_square_root(image_height), get_texture_format(color_type), texture_data);
		else output_texture = create_texture(image_width, image_height, get_texture_format(color_type), texture_data);

		delete[] texture_row_pointers;
		delete[] texture_data;

		png_destroy_read_struct(&png_struct, &png_info, nullptr);
	}
	else
	{
		XPLMDebugString("Could not open texture file!");

		XPLMDebugString("Texture file path is:");
		XPLMDebugString(texture_path);
	}

	return output_texture;
}

GLuint load_shader(const char* shader_path, GLenum shader_type)
{
	GLuint output_shader = EMPTY_OBJECT;

	std::ifstream shader_file(shader_path, std::ifstream::binary | std::ifstream::ate);

	if (shader_file.fail() == false)
	{
		GLint shader_file_size = shader_file.tellg();

		shader_file.seekg(0);

		GLchar* shader_string = new GLchar[shader_file_size];
		shader_file.read(shader_string, shader_file_size);

		GLuint shader_reference = glCreateShader(shader_type);

		glShaderSource(shader_reference, 1, &shader_string, &shader_file_size);
		glCompileShader(shader_reference);

		delete[] shader_string;

		GLint shader_compilation_status;
		glGetShaderiv(shader_reference, GL_COMPILE_STATUS, &shader_compilation_status);

		if (shader_compilation_status == GL_TRUE) output_shader = shader_reference;
		else
		{
			GLint compilation_log_length;
			glGetShaderiv(shader_reference, GL_INFO_LOG_LENGTH, &compilation_log_length);

			GLchar* compilation_message = new GLchar[compilation_log_length];
			glGetShaderInfoLog(shader_reference, compilation_log_length, nullptr, compilation_message);

			XPLMDebugString("\nShader compilation failed!\n\n");

			XPLMDebugString("Compilation error message is:\n");
			XPLMDebugString(compilation_message);

			delete[] compilation_message;
		}
	}
	else
	{
		XPLMDebugString("\nCould not open shader file!\n\n");

		XPLMDebugString("Shader file path is:\n");
		XPLMDebugString(shader_path);
	}

	return output_shader;
}

GLuint create_program(GLuint vertex_shader, GLuint fragment_shader)
{
	GLuint output_program = EMPTY_OBJECT;

	GLuint program_reference = glCreateProgram();

	glAttachShader(program_reference, vertex_shader);
	glAttachShader(program_reference, fragment_shader);

	glLinkProgram(program_reference);

	GLint program_link_status;
	glGetProgramiv(program_reference, GL_LINK_STATUS, &program_link_status);

	if (program_link_status == GL_TRUE) output_program = program_reference;
	else
	{
		GLint compilation_log_length;
		glGetProgramiv(program_reference, GL_INFO_LOG_LENGTH, &compilation_log_length);

		GLchar* compilation_message = new GLchar[compilation_log_length];
		glGetProgramInfoLog(program_reference, compilation_log_length, nullptr, compilation_message);

		XPLMDebugString("\nProgram compilation failed!\n\n");

		XPLMDebugString("Compilation error message is:\n");
		XPLMDebugString(compilation_message);

		delete[] compilation_message;
	}

	return output_program;
}