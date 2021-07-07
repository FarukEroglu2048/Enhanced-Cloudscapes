#version 450 core

in vec2 fullscreen_texture_position;

uniform sampler2D current_rendering_texture;

layout(location = 0) out vec4 fragment_color;

#define CONSTANT_1 2.51
#define CONSTANT_2 0.03
#define CONSTANT_3 2.43
#define CONSTANT_4 0.59
#define CONSTANT_5 0.14

vec3 tone_mapping(vec3 input_color)
{
	return (input_color * ((CONSTANT_1 * input_color) + CONSTANT_2)) / ((input_color * ((CONSTANT_3 * input_color) + CONSTANT_4)) + CONSTANT_5);
}

void main()
{
	vec4 rendered_color = texture(current_rendering_texture, fullscreen_texture_position);
	rendered_color.xyz = tone_mapping(rendered_color.xyz);

	fragment_color = rendered_color;
}