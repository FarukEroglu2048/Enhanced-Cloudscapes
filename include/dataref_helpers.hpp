#pragma once

#include <XPLMDataAccess.h>

#include <glm/vec3.hpp>

XPLMDataRef export_float_dataref(char* dataref_name, float initial_value);
XPLMDataRef export_vec3_dataref(char* dataref_name, glm::vec3 initial_value);