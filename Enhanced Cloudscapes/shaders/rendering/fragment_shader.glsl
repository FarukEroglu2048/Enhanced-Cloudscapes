#version 450 core

#define EARTH_RADIUS 6378100.0
#define EARTH_CENTER vec3(0.0, -1.0 * EARTH_RADIUS, 0.0)

#define SAMPLE_STEP_COUNT 64
#define SUN_STEP_COUNT 6

#define MAXIMUM_SAMPLE_STEP_SIZE 100.0

#define CLOUD_LAYER_COUNT 3
#define CLOUD_TYPE_COUNT 5

#define PI 3.141592653589793

in vec3 ray_start_position;
in vec3 ray_end_position;

in vec2 fullscreen_texture_position;

uniform sampler2D depth_texture;

uniform sampler2D[CLOUD_LAYER_COUNT] cloud_map_textures;

uniform sampler3D base_noise_texture;
uniform sampler3D detail_noise_texture;

uniform sampler2D blue_noise_texture;

uniform sampler2D rendering_texture;

uniform int skip_fragments;
uniform int frame_index;

uniform float near_clip_z;
uniform float far_clip_z;

uniform mat4 inverse_projection_matrix;
uniform mat4 inverse_modelview_matrix;

uniform float cloud_map_scale;

uniform float base_noise_scale;
uniform float detail_noise_scale;

uniform float blue_noise_scale;

uniform int[CLOUD_LAYER_COUNT] cloud_types;

uniform float[CLOUD_LAYER_COUNT] cloud_bases;
uniform float[CLOUD_LAYER_COUNT] cloud_tops;

uniform float[CLOUD_TYPE_COUNT] cloud_coverages;
uniform float[CLOUD_TYPE_COUNT] cloud_densities;

uniform vec3[CLOUD_TYPE_COUNT] base_noise_ratios;
uniform vec3[CLOUD_TYPE_COUNT] detail_noise_ratios;

uniform vec3[CLOUD_TYPE_COUNT] wind_offsets;

uniform float fade_start_distance;
uniform float fade_end_distance;

uniform vec3 sun_direction;

uniform vec3 sun_tint;
uniform float sun_gain;

uniform vec3 ambient_tint;
uniform float ambient_gain;

uniform float forward_mie_scattering;
uniform float backward_mie_scattering;

uniform vec3 atmosphere_bottom_tint;
uniform vec3 atmosphere_top_tint;

uniform float atmospheric_blending;

layout(location = 0) out vec4 fragment_color;

float map(in float input_value, in float input_start, in float input_end, in float output_start, in float output_end)
{
	float slope = (output_end - output_start) / (input_end - input_start);

	return clamp((slope * (input_value - input_start)) + output_start, min(output_start, output_end), max(output_start, output_end));
}

float henyey_greenstein(in float dot_angle, in float scattering_value)
{
	float squared_scattering_value = pow(scattering_value, 2.0);

	return (1.0 - squared_scattering_value) / (4.0 * PI * pow(squared_scattering_value - (2.0 * scattering_value * dot_angle) + 1.0, 1.5));
}

float get_height_ratio(in vec3 ray_position, in int cloud_layer_index)
{
	return map(length(ray_position - EARTH_CENTER) - EARTH_RADIUS, cloud_bases[cloud_layer_index], cloud_tops[cloud_layer_index], 0.0, 1.0);
}

float sample_clouds(in vec3 ray_position, in int cloud_layer_index)
{
	vec4 base_noise_sample = texture(base_noise_texture, (ray_position + wind_offsets[cloud_layer_index]) * base_noise_scale);
	float base_noise = map(base_noise_sample.x, dot(base_noise_sample.yzw, base_noise_ratios[cloud_types[cloud_layer_index] - 1]), 1.0, 0.0, 1.0);

	vec2 cloud_map_sample = texture(cloud_map_textures[cloud_layer_index], (ray_position.xz + wind_offsets[cloud_layer_index].xz) * cloud_map_scale).xy;

	if (cloud_types[cloud_layer_index] == 1) cloud_map_sample.y = 1.0;
	else cloud_map_sample.y = map(cloud_map_sample.y, 0.0, 1.0, 0.625, 1.0);

	float height_ratio = get_height_ratio(ray_position, cloud_layer_index);
	float height_multiplier = min(map(height_ratio, 0.0, 0.125, 0.0, 1.0), map(height_ratio, 0.625 * cloud_map_sample.y, cloud_map_sample.y, 1.0, 0.0));

	float base_erosion = map(base_noise * height_multiplier, 1.0 - max(cloud_map_sample.x, cloud_coverages[cloud_types[cloud_layer_index] - 1]), 1.0, 0.0, 1.0);

	if (base_erosion > 0.01)
	{
		vec3 detail_noise_sample = texture(detail_noise_texture, ray_position * detail_noise_scale).xyz;
		float detail_noise = dot(detail_noise_sample, detail_noise_ratios[cloud_types[cloud_layer_index] - 1]);

		return map(base_erosion, 0.625 * detail_noise, 1.0, 0.0, 1.0) * cloud_densities[cloud_types[cloud_layer_index] - 1] * (cloud_coverages[cloud_types[cloud_layer_index] - 1] + 1.0) * map(length(ray_position - ray_start_position), fade_start_distance, fade_end_distance, 1.0, 0.0);
	}
	else return 0.0;
}

vec2 ray_sphere_intersections(in vec3 ray_position, in vec3 ray_direction, in float sphere_height)
{
	vec3 ray_earth_vector = ray_position - EARTH_CENTER;

	float coefficient_1 = 2.0 * dot(ray_direction, ray_earth_vector);
	float coefficient_2 = dot(ray_earth_vector, ray_earth_vector) - pow(EARTH_RADIUS + sphere_height, 2.0);

	float discriminant = pow(coefficient_1, 2.0) - (4.0 * coefficient_2);

	if (discriminant < 0.0) return vec2(0.0, 0.0);
	else
	{
		float lower_solution = ((-1.0 * coefficient_1) - sqrt(discriminant)) / 2.0;
		float higher_solution = ((-1.0 * coefficient_1) + sqrt(discriminant)) / 2.0;

		if (lower_solution < 0.0) return vec2(max(higher_solution, 0.0), 0.0);
		else return vec2(lower_solution, higher_solution);
	}
}

vec3 get_world_intersection()
{
	vec4 world_vector = inverse_projection_matrix * vec4((fullscreen_texture_position * 2.0) - 1.0, map(texture(depth_texture, fullscreen_texture_position).x, 0.0, 1.0, min(near_clip_z, far_clip_z), max(near_clip_z, far_clip_z)), 1.0);
	world_vector /= world_vector.w;

	return vec3(inverse_modelview_matrix * world_vector);
}

vec2 ray_layer_intersections(in vec3 ray_position, in vec3 ray_direction, int cloud_layer_index)
{
	vec2 inner_sphere_intersections = ray_sphere_intersections(ray_position, ray_direction, cloud_bases[cloud_layer_index]);
	vec2 outer_sphere_intersections = ray_sphere_intersections(ray_position, ray_direction, cloud_tops[cloud_layer_index]);

	vec3 world_intersection = get_world_intersection();
	float world_distance = length(world_intersection - ray_position);

	float height_ratio = get_height_ratio(ray_position, cloud_layer_index);

	float ray_start_distance = 0.0;
	float ray_march_distance = 0.0;

	if (height_ratio == 0.0)
	{
		ray_start_distance = inner_sphere_intersections.x;
		ray_march_distance = outer_sphere_intersections.x - inner_sphere_intersections.x;
	}
	else if ((height_ratio > 0.0) && (height_ratio < 1.0))
	{
		float lower_distance = min(inner_sphere_intersections.x, outer_sphere_intersections.x);
		float higher_distance = max(inner_sphere_intersections.x, outer_sphere_intersections.x);

		if (lower_distance == 0.0) ray_march_distance = higher_distance;
		else ray_march_distance = lower_distance;
	}
	else if (height_ratio == 1.0)
	{
		if (inner_sphere_intersections.x == 0.0)
		{
			ray_start_distance = outer_sphere_intersections.x;
			ray_march_distance = outer_sphere_intersections.y - outer_sphere_intersections.x;
		}
		else
		{
			ray_start_distance = outer_sphere_intersections.x;
			ray_march_distance = inner_sphere_intersections.x - outer_sphere_intersections.x;
		}
	}

	ray_start_distance = min(ray_start_distance, world_distance);
	ray_march_distance = min(min(ray_march_distance, world_distance - ray_start_distance), fade_end_distance - ray_start_distance);

	return vec2(ray_start_distance, ray_march_distance);
}

int get_first_higher_layer(in vec3 ray_position)
{
	int first_higher_layer = 0;
	while ((first_higher_layer < CLOUD_LAYER_COUNT) && (get_height_ratio(ray_position, first_higher_layer) != 0.0)) first_higher_layer++;

	return first_higher_layer;
}

float sun_ray_march(in float input_transmittance, in vec3 ray_position, in int cloud_layer_index)
{
	float output_transmittance = input_transmittance;

	if (cloud_types[cloud_layer_index] != 0)
	{
		float step_size = (cloud_tops[cloud_layer_index] - ray_position.y) / SUN_STEP_COUNT;

		vec3 current_ray_position = ray_position;

		for (int step_index = 0; step_index < SUN_STEP_COUNT; step_index++)
		{
			float cloud_sample = sample_clouds(current_ray_position, cloud_layer_index);

			output_transmittance *= exp(-1.0 * cloud_sample * step_size);
			if (output_transmittance < 0.01) break;

			current_ray_position += sun_direction * step_size;
		}
	}

	return output_transmittance;
}

vec4 sample_ray_march(in vec4 input_color, in int cloud_layer_index)
{
	vec4 output_color = input_color;
	
	if (cloud_types[cloud_layer_index] != 0)
	{
		vec3 ray_direction = normalize(ray_end_position - ray_start_position);

		vec2 layer_intersections = ray_layer_intersections(ray_start_position, ray_direction, cloud_layer_index);

		if (layer_intersections.y > 5.0)
		{
			vec3 current_ray_position = ray_start_position + (ray_direction * layer_intersections.x);
			float current_ray_distance = 0.0;

			float step_size = min(layer_intersections.y / SAMPLE_STEP_COUNT, MAXIMUM_SAMPLE_STEP_SIZE);

			float sun_angle_multiplier = map(sun_direction.y, 0.05, -0.125, 1.0, 0.125);

			float sun_dot_angle = dot(ray_direction, sun_direction);
			float mie_scattering_gain = clamp(mix(henyey_greenstein(sun_dot_angle, forward_mie_scattering), henyey_greenstein(sun_dot_angle, -1.0 * backward_mie_scattering), 0.5), 1.0, 2.5);

			while (current_ray_distance <= layer_intersections.y)
			{
				float current_step_size = step_size * map(texture(blue_noise_texture, current_ray_position.xz * blue_noise_scale).x, 0.0, 1.0, 0.75, 1.0) * map(current_ray_distance, 0.0, layer_intersections.y, 1.0, 8.0);

				float cloud_sample = sample_clouds(current_ray_position, cloud_layer_index);

				if (cloud_sample != 0.0)
				{
					float light_attenuation = 1.0;
					for (int current_layer_index = cloud_layer_index; current_layer_index < CLOUD_LAYER_COUNT; current_layer_index++) light_attenuation = sun_ray_march(light_attenuation, current_ray_position + (ray_layer_intersections(current_ray_position, sun_direction, current_layer_index).x * sun_direction), current_layer_index);

					vec3 sun_color = sun_tint * sun_gain * mie_scattering_gain * light_attenuation * sun_angle_multiplier;
					vec3 ambient_color = mix(ambient_tint, mix(atmosphere_bottom_tint, atmosphere_top_tint, get_height_ratio(current_ray_position, cloud_layer_index)) * dot(ambient_tint, vec3(0.21, 0.72, 0.07)), atmospheric_blending) * ambient_gain * sun_angle_multiplier;

					vec3 sample_color = (ambient_color + sun_color) * cloud_sample * current_step_size;
					float sample_transmittance = exp(-1.0 * cloud_sample * current_step_size);

					output_color.xyz += sample_color * output_color.w;
					output_color.w *= sample_transmittance;

					if (output_color.w < 0.01) break;
				}

				current_ray_position += ray_direction * current_step_size;
				current_ray_distance += current_step_size;
			}
		}
	}

	return output_color;
}

vec4 render_clouds()
{
	vec4 output_color = vec4(0.0, 0.0, 0.0, 1.0);

	int first_higher_layer = get_first_higher_layer(ray_start_position);

	for (int cloud_layer_index = first_higher_layer - 1; cloud_layer_index >= 0; cloud_layer_index--) output_color = sample_ray_march(output_color, cloud_layer_index);
	for (int cloud_layer_index = first_higher_layer; cloud_layer_index < CLOUD_LAYER_COUNT; cloud_layer_index++) output_color = sample_ray_march(output_color, cloud_layer_index);

	vec3 world_intersection = get_world_intersection();

	float shadow_attenuation = 1.0;
	for (int cloud_layer_index = get_first_higher_layer(world_intersection); cloud_layer_index < CLOUD_LAYER_COUNT; cloud_layer_index++) shadow_attenuation = sun_ray_march(shadow_attenuation, world_intersection + (ray_layer_intersections(world_intersection, sun_direction, cloud_layer_index).x * sun_direction), cloud_layer_index);

	output_color.xyz = mix(output_color.xyz, 0.05 * atmosphere_bottom_tint, output_color.w * (1.0 - shadow_attenuation));
	output_color.w *= map(shadow_attenuation, 0.0, 1.0, 0.25, 1.0);

	output_color.w = 1.0 - output_color.w;
	output_color.xyz /= output_color.w;

	return output_color;
}

void main()
{
	if ((skip_fragments != 0) && ((frame_index % 2) == (int(gl_FragCoord.x) % 2))) fragment_color = texture(rendering_texture, fullscreen_texture_position);
	else fragment_color = render_clouds();
}