#version 450 core

layout(location = 0) in vec2 input_vertex;

uniform float near_clip_z;

out vec2 fullscreen_texture_position;

void main()
{
	fullscreen_texture_position = (input_vertex / 2.0) + 0.5;
	gl_Position = vec4(input_vertex, near_clip_z, 1.0);
}