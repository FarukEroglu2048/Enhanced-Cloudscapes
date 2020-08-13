#include <dataref_helpers.hpp>

int read_int_callback(void* float_pointer)
{
	return *static_cast<int*>(float_pointer);
}

void write_int_callback(void* float_pointer, int new_value)
{
	*static_cast<int*>(float_pointer) = new_value;
}

float read_float_callback(void* float_pointer)
{
	return *static_cast<float*>(float_pointer);
}

void write_float_callback(void* float_pointer, float new_value)
{
	*static_cast<float*>(float_pointer) = new_value;
}

int read_vec3_callback(void* vec3_pointer, float* output_values, int read_offset, int read_size)
{
	glm::vec3& input_vec3 = *static_cast<glm::vec3*>(vec3_pointer);

	if (output_values != nullptr)
	{
		int read_start = read_offset % input_vec3.length();
		int read_end = read_start + read_size;

		if (read_end < read_start) read_end = read_start;
		else if (read_end > input_vec3.length()) read_end = input_vec3.length();

		for (int index = read_start; index < read_end; index++) output_values[index] = input_vec3[index];

		return read_end - read_start;
	}
	else return input_vec3.length();
}

void write_vec3_callback(void* vector_pointer, float* input_values, int write_offset, int write_size)
{
	glm::vec3& output_vec3 = *static_cast<glm::vec3*>(vector_pointer);

	int write_start = write_offset % output_vec3.length();
	int write_end = write_start + write_size;

	if (write_end < write_start) write_end = write_start;
	else if (write_end > output_vec3.length()) write_end = output_vec3.length();

	for (int index = write_start; index < write_end; index++) output_vec3[index] = input_values[index];
}

XPLMDataRef export_int_dataref(char* dataref_name, int initial_value)
{
	int* int_pointer = new int(initial_value);

	return XPLMRegisterDataAccessor(dataref_name, xplmType_Int, 1, read_int_callback, write_int_callback, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, int_pointer, int_pointer);
}

XPLMDataRef export_float_dataref(char* dataref_name, float initial_value)
{
	float* float_pointer = new float(initial_value);

	return XPLMRegisterDataAccessor(dataref_name, xplmType_Float, 1, nullptr, nullptr, read_float_callback, write_float_callback, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, float_pointer, float_pointer);
}

XPLMDataRef export_vec3_dataref(char* dataref_name, glm::vec3 initial_value)
{
	glm::vec3* vec3_pointer = new glm::vec3(initial_value);

	return XPLMRegisterDataAccessor(dataref_name, xplmType_FloatArray, 1, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, read_vec3_callback, write_vec3_callback, nullptr, nullptr, vec3_pointer, vec3_pointer);
}