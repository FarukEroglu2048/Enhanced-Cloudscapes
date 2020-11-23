#version 450 core

#define CLOUD_LAYER_COUNT 3
#define CLOUD_TYPE_COUNT 6

#define EARTH_RADIUS 6378100.0
#define EARTH_CENTER vec3(0.0, -1.0 * EARTH_RADIUS, 0.0)

#define PI 3.141592653589793

in vec3 ray_start_position;
in vec3 ray_end_position;

in vec2 fullscreen_texture_position;

uniform sampler2D previous_depth_texture;
uniform sampler2D current_depth_texture;

uniform sampler2D[CLOUD_LAYER_COUNT] cloud_map_textures;

uniform sampler3D base_noise_texture;
uniform sampler3D detail_noise_texture;

uniform sampler2D blue_noise_texture;

uniform sampler2D previous_rendering_texture;

uniform int skip_fragments;
uniform int frame_index;

uniform int sample_step_count;
uniform int sun_step_count;

uniform float maximum_sample_step_size;
uniform float maximum_sun_step_size;

uniform int use_blue_noise_dithering;

uniform float near_clip_z;
uniform float far_clip_z;

uniform mat4 previous_mvp_matrix;
uniform mat4 current_mvp_matrix;

uniform mat4 inverse_modelview_matrix;
uniform mat4 inverse_projection_matrix;

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

uniform float base_anvil;
uniform float top_anvil;

uniform float fade_start_distance;
uniform float fade_end_distance;

uniform float light_attenuation;

uniform vec3 sun_direction;

uniform vec3 sun_tint;
uniform float sun_gain;

uniform vec3 ambient_tint;
uniform float ambient_gain;

uniform float mie_scattering;

uniform vec3 atmosphere_bottom_tint;
uniform vec3 atmosphere_top_tint;

uniform float atmospheric_blending;

layout(location = 0) out vec4 fragment_color;

vec3 sample_direction;

vec3 world_intersection;
float world_distance;

int bayer_filter[16] =
{
	0, 8, 2, 10,
	12, 4, 14, 6,
	3, 11, 1, 9,
	15, 7, 13, 5
};

float map(in float input_value, in float input_start, in float input_end, in float output_start, in float output_end)
{
	float slope = (output_end - output_start) / (input_end - input_start);

	return clamp((slope * (input_value - input_start)) + output_start, min(output_start, output_end), max(output_start, output_end));
}

float get_height_ratio(in vec3 ray_position, in int layer_index)
{
	return map(length(ray_position - EARTH_CENTER) - EARTH_RADIUS, cloud_bases[layer_index], cloud_tops[layer_index], 0.0, 1.0);
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

vec2 ray_layer_intersections(in vec3 ray_position, in vec3 ray_direction, in int layer_index)
{
	vec2 layer_intersections = vec2(0.0, 0.0);

	vec2 inner_sphere_intersections = ray_sphere_intersections(ray_position, ray_direction, cloud_bases[layer_index]);
	vec2 outer_sphere_intersections = ray_sphere_intersections(ray_position, ray_direction, cloud_tops[layer_index]);

	float height_ratio = get_height_ratio(ray_position, layer_index);

	if (height_ratio == 0.0)
	{
		layer_intersections.x = inner_sphere_intersections.x;
		layer_intersections.y = outer_sphere_intersections.x - inner_sphere_intersections.x;
	}
	else if ((height_ratio > 0.0) && (height_ratio < 1.0))
	{
		float lower_distance = min(inner_sphere_intersections.x, outer_sphere_intersections.x);
		float higher_distance = max(inner_sphere_intersections.x, outer_sphere_intersections.x);

		if (lower_distance == 0.0) layer_intersections.y = higher_distance;
		else layer_intersections.y = lower_distance;
	}
	else if (height_ratio == 1.0)
	{
		if (inner_sphere_intersections.x == 0.0)
		{
			layer_intersections.x = outer_sphere_intersections.x;
			layer_intersections.y = outer_sphere_intersections.y - outer_sphere_intersections.x;
		}
		else
		{
			layer_intersections.x = outer_sphere_intersections.x;
			layer_intersections.y = inner_sphere_intersections.x - outer_sphere_intersections.x;
		}
	}

	return layer_intersections;
}

int get_closest_layer(in vec3 ray_position, in vec3 ray_direction)
{
	int closest_layer;

	if (ray_direction.y < 0.0)
	{
		closest_layer = CLOUD_LAYER_COUNT - 1;
		while ((closest_layer > 0) && ((cloud_types[closest_layer] == 0) || (get_height_ratio(ray_position, closest_layer) == 0.0))) closest_layer--;
	}
	else
	{
		closest_layer = 0;
		while ((closest_layer < (CLOUD_LAYER_COUNT - 1)) && ((cloud_types[closest_layer] == 0) || (get_height_ratio(ray_position, closest_layer) == 1.0))) closest_layer++;
	}

	return closest_layer;
}

float henyey_greenstein(in float dot_angle, in float scattering_value)
{
	float squared_scattering_value = pow(scattering_value, 2.0);

	return (1.0 - squared_scattering_value) / (4.0 * PI * pow(squared_scattering_value - (2.0 * scattering_value * dot_angle) + 1.0, 1.5));
}

float sample_clouds(in vec3 ray_position, in int layer_index)
{
	vec4 base_noise_sample = texture(base_noise_texture, (ray_position + wind_offsets[layer_index]) * base_noise_scale);
	float base_noise = map(base_noise_sample.x, dot(base_noise_sample.yzw, base_noise_ratios[cloud_types[layer_index] - 1]), 1.0, 0.0, 1.0);

	vec2 cloud_map_sample = texture(cloud_map_textures[layer_index], (ray_position.xz + wind_offsets[layer_index].xz) * cloud_map_scale).xy;

	if (cloud_types[layer_index] == 1) cloud_map_sample.y = 1.0;
	else cloud_map_sample.y = map(cloud_map_sample.y, 0.0, 1.0, 0.25, 1.0);

	float height_ratio = get_height_ratio(ray_position, layer_index);
	float height_multiplier = min(map(height_ratio, 0.0, 0.125, 0.0, 1.0), map(height_ratio, 0.625 * cloud_map_sample.y, cloud_map_sample.y, 1.0, 0.0));

	float anvil_multiplier = max(map(height_ratio, 0.125, 0.25, base_anvil, 1.0), map(height_ratio, 0.5 * cloud_map_sample.y, 0.625 * cloud_map_sample.y, 1.0, top_anvil));
	float cloud_coverage = clamp(max(cloud_map_sample.x, cloud_coverages[cloud_types[layer_index] - 1]) * anvil_multiplier, 0.0, 1.0);

	float base_erosion = map(base_noise * height_multiplier, 1.0 - cloud_coverage, 1.0, 0.0, 1.0);

	if (base_erosion > 0.01)
	{
		vec3 detail_noise_sample = texture(detail_noise_texture, ray_position * detail_noise_scale).xyz;
		float detail_noise = dot(detail_noise_sample, detail_noise_ratios[cloud_types[layer_index] - 1]);

		return map(base_erosion, 0.75 * detail_noise, 1.0, 0.0, 1.0) * cloud_densities[cloud_types[layer_index] - 1] * map(cloud_coverages[cloud_types[layer_index] - 1], 0.0, 1.0, 1.0, 1.5);
	}
	else return 0.0;
}

float sun_ray_march(in float input_transmittance, in vec3 ray_position, in int layer_index)
{
	float output_transmittance = input_transmittance;

	if (cloud_types[layer_index] != 0)
	{
		vec2 layer_intersections = ray_layer_intersections(ray_position, sun_direction, layer_index);

		vec3 current_ray_position = ray_position + (sun_direction * layer_intersections.x);
		float step_size = min(layer_intersections.y / float(sun_step_count), maximum_sun_step_size);

		for (int step_index = 0; step_index < sun_step_count; step_index++)
		{
			output_transmittance *= exp(-1.0 * light_attenuation * sample_clouds(current_ray_position, layer_index) * step_size);
			if (output_transmittance < 0.01) break;

			current_ray_position += sun_direction * step_size;
		}
	}

	return output_transmittance;
}

vec4 sample_ray_march(in vec4 input_color, in int layer_index)
{
	vec4 output_color = input_color;

	if (cloud_types[layer_index] != 0)
	{
		vec2 layer_intersections = ray_layer_intersections(ray_start_position, sample_direction, layer_index);

		float maximum_distance = min(world_distance, fade_end_distance * map(abs(ray_start_position.y - cloud_bases[layer_index]), 0.0, 15240.0, 1.0, 24.0));

		layer_intersections.x = min(layer_intersections.x, maximum_distance);
		layer_intersections.y = min(layer_intersections.y, maximum_distance - layer_intersections.x);

		if (layer_intersections.y > 5.0)
		{
			vec3 current_ray_position = ray_start_position + (sample_direction * layer_intersections.x);
			float current_ray_distance = 0.0;

			float step_size = min(layer_intersections.y / float(sample_step_count), maximum_sample_step_size);

			float sun_angle_multiplier = map(sun_direction.y, 0.05, -0.125, 1.0, 0.175);

			float sun_dot_angle = dot(sample_direction, sun_direction);
			float mie_scattering_gain = clamp(henyey_greenstein(sun_dot_angle, mie_scattering) + henyey_greenstein(sun_dot_angle, mie_scattering * 0.75) + henyey_greenstein(sun_dot_angle, mie_scattering * 0.5), 1.0, 2.75);

			while (current_ray_distance <= layer_intersections.y)
			{
				float distance_multiplier;

				if (current_ray_distance < 40000.0) distance_multiplier = map(current_ray_distance, 0.0, 40000.0, 1.0, 4.0);
				else distance_multiplier = map(current_ray_distance, 40000.0, layer_intersections.y, 4.0, 16.0);

				float current_step_size = step_size * distance_multiplier;
				if (use_blue_noise_dithering != 0) current_step_size *= map(texture(blue_noise_texture, current_ray_position.xz * blue_noise_scale).x, 0.0, 1.0, 0.75, 1.0);

				float cloud_sample = sample_clouds(current_ray_position, layer_index);

				if (cloud_sample != 0.0)
				{
					float sample_attenuation = 1.0;

					if (sun_direction.y < 0.0)
					{
						for (int current_layer_index = layer_index; current_layer_index >= 0; current_layer_index--) sample_attenuation = sun_ray_march(sample_attenuation, current_ray_position, current_layer_index);
					}
					else
					{
						for (int current_layer_index = layer_index; current_layer_index < CLOUD_LAYER_COUNT; current_layer_index++) sample_attenuation = sun_ray_march(sample_attenuation, current_ray_position, current_layer_index);
					}

					vec3 sun_color = sun_tint * sun_gain * mie_scattering_gain * sample_attenuation * sun_angle_multiplier;
					vec3 ambient_color = mix(ambient_tint, mix(atmosphere_bottom_tint, atmosphere_top_tint, get_height_ratio(current_ray_position, layer_index)) * dot(ambient_tint, vec3(0.21, 0.72, 0.07)), atmospheric_blending) * ambient_gain * sun_angle_multiplier;

					vec3 sample_color = ambient_color + sun_color;
					float sample_transmittance = exp(-1.0 * cloud_sample * current_step_size);

					output_color.xyz += (sample_color - (sample_color * sample_transmittance)) * output_color.w;
					output_color.w *= sample_transmittance;
					
					if (output_color.w < 0.01) break;
				}

				current_ray_position += sample_direction * current_step_size;
				current_ray_distance += current_step_size;
			}
		}
	}

	return output_color;
}

void main()
{
	sample_direction = normalize(ray_end_position - ray_start_position);

	vec4 world_vector = inverse_projection_matrix * vec4((fullscreen_texture_position * 2.0) - 1.0, map(texture(current_depth_texture, fullscreen_texture_position).x, 0.0, 1.0, min(near_clip_z, far_clip_z), max(near_clip_z, far_clip_z)), 1.0);
	world_vector /= world_vector.w;

	world_intersection = vec3(inverse_modelview_matrix * world_vector);
	world_distance = length(world_intersection - ray_start_position);

	vec4 previous_fragment_vector = previous_mvp_matrix * vec4(ray_end_position, 1.0);
	previous_fragment_vector /= previous_fragment_vector.w;

	bool skip_fragment;

	if (skip_fragments != 0)
	{
		ivec2 fragment_indices = ivec2(gl_FragCoord.xy) % 4;
		int fragment_index = (fragment_indices.y * 4) + fragment_indices.x;

		if (fragment_index != bayer_filter[frame_index % 16])
		{
			if ((abs(previous_fragment_vector.x) < 1.0) && (abs(previous_fragment_vector.y) < 1.0))
			{
				if (abs(texture(current_depth_texture, fullscreen_texture_position).x - texture(previous_depth_texture, fullscreen_texture_position).x) < 0.001) skip_fragment = true;
				else skip_fragment = false;
			}
			else skip_fragment = false;
		}
		else skip_fragment = false;
	}
	else skip_fragment = false;

	if (skip_fragment == false)
	{
		vec4 output_color = vec4(0.0, 0.0, 0.0, 1.0);

		int closest_layer = get_closest_layer(ray_start_position, sample_direction);
		int world_closest_layer = get_closest_layer(world_intersection, sun_direction);

		if (sample_direction.y < 0.0)
		{
			for (int layer_index = closest_layer; layer_index >= 0; layer_index--) output_color = sample_ray_march(output_color, layer_index);
		}
		else
		{
			for (int layer_index = closest_layer; layer_index < CLOUD_LAYER_COUNT; layer_index++) output_color = sample_ray_march(output_color, layer_index);
		}

		if (get_height_ratio(ray_start_position, closest_layer) == 0.0)
		{
			float altitude_multiplier = map(cloud_bases[closest_layer] - ray_start_position.y, 0.0, 15240.0, 1.0, 24.0);

			float ray_end_distance = length(ray_end_position - ray_start_position);
			float ray_end_ratio = min(ray_end_distance / (fade_end_distance * altitude_multiplier), 1.0);

			output_color.w = mix(output_color.w, 1.0, map(ray_layer_intersections(ray_start_position, sample_direction, closest_layer).x, fade_start_distance * altitude_multiplier * ray_end_ratio, fade_end_distance * altitude_multiplier * ray_end_ratio, 0.0, 1.0));
		}

		float shadow_attenuation = 1.0;

		if (sun_direction.y < 0.0)
		{
			for (int layer_index = world_closest_layer; layer_index >= 0; layer_index--) shadow_attenuation = sun_ray_march(shadow_attenuation, world_intersection, layer_index);
		}
		else
		{
			for (int layer_index = world_closest_layer; layer_index < CLOUD_LAYER_COUNT; layer_index++) shadow_attenuation = sun_ray_march(shadow_attenuation, world_intersection, layer_index);
		}

		shadow_attenuation = mix(map(shadow_attenuation, 0.0, 1.0, 0.25, 1.0), 1.0, max(map(length(ray_start_position - world_intersection), fade_start_distance, fade_end_distance, 0.0, 1.0), map(sun_direction.y, 0.15, 0.0, 0.0, 1.0)));

		output_color.xyz += mix(vec3(0.0, 0.0, 0.0), atmosphere_bottom_tint, 0.1) * (1.0 - shadow_attenuation) * output_color.w;
		output_color.w *= shadow_attenuation;

		output_color.w = 1.0 - output_color.w;
		fragment_color = vec4(output_color.xyz / output_color.w, output_color.w);
	}
	else fragment_color = texture(previous_rendering_texture, (previous_fragment_vector.xy / 2.0) + 0.5);
}