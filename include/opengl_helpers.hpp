#pragma once

#include <GL/glew.h>

#define EMPTY_OBJECT 0

int create_fullscreen_texture();

int create_texture(GLsizei texture_width, GLsizei texture_height, GLenum data_format, const void* texture_data);
int create_texture(GLsizei texture_width, GLsizei texture_height, GLsizei texture_depth, GLenum data_format, const void* texture_data);

int load_png_texture(const char* texture_path, bool add_depth);

GLuint load_shader(const char* shader_path, GLenum shader_type);

GLuint create_program(GLuint vertex_shader, GLuint fragment_shader);