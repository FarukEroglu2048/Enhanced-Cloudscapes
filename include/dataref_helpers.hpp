#pragma once

#include <XPLMDataAccess.h>

#include <glm/vec3.hpp>
#include <vector>
#include <unordered_map>
#include <string>
XPLMDataRef export_int_dataref(char* dataref_name, int initial_value);
XPLMDataRef export_float_dataref(char* dataref_name, float initial_value);
XPLMDataRef export_vec3_dataref(char* dataref_name, glm::vec3 initial_value);