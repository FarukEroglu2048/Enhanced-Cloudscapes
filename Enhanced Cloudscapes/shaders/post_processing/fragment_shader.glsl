#version 450 core

in vec2 fullscreen_texture_position;

uniform sampler2D rendering_texture;

layout(location = 0) out vec4 fragment_color;

void main()
{
	vec4 rendered_color = texture(rendering_texture, fullscreen_texture_position);
	rendered_color.xyz = 1.0 - exp(-1.0 * rendered_color.xyz);

	fragment_color = rendered_color;
}