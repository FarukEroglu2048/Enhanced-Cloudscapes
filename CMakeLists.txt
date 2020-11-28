cmake_minimum_required(VERSION 3.8)

project("Enhanced_Cloudscapes")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(XPLM_SDK_INCLUDE_DIRECTORY "" CACHE PATH "X-Plane SDK include directory")
set(XPLM_SDK_LIBRARY_FILE "" CACHE FILEPATH "X-Plane SDK library file (leave empty on Linux)")

find_package(OpenGL REQUIRED)

set(GLEW_INCLUDE_DIRECTORY "" CACHE PATH "GLEW include directory")
set(GLEW_LIBRARY_FILE "" CACHE FILEPATH "GLEW library file")

set(ZLIB_INCLUDE_DIRECTORY "" CACHE PATH "zlib include directory")
set(ZLIB_LIBRARY_FILE "" CACHE FILEPATH "zlib library file")

set(LIBPNG_INCLUDE_DIRECTORY "" CACHE PATH "libpng include directory")
set(LIBPNG_LIBRARY_FILE "" CACHE FILEPATH "libpng library file")

set(GLM_INCLUDE_DIRECTORY "" CACHE PATH "GLM include directory")

add_library(enhanced_cloudscapes SHARED "include/dataref_helpers.hpp" "include/opengl_helpers.hpp" "include/simulator_objects.hpp" "include/plugin_objects.hpp" "include/rendering_program.hpp" "include/post_processing_program.hpp" "src/dataref_helpers.cpp" "src/opengl_helpers.cpp" "src/simulator_objects.cpp" "src/plugin_objects.cpp" "src/rendering_program.cpp" "src/post_processing_program.cpp" "src/enhanced_cloudscapes.cpp")

target_include_directories(enhanced_cloudscapes PRIVATE "include" ${XPLM_SDK_INCLUDE_DIRECTORY} ${GLEW_INCLUDE_DIRECTORY} ${ZLIB_INCLUDE_DIRECTORY} ${LIBPNG_INCLUDE_DIRECTORY} ${GLM_INCLUDE_DIRECTORY})
target_link_libraries(enhanced_cloudscapes PRIVATE ${XPLM_SDK_LIBRARY_FILE} OpenGL::GL ${GLEW_LIBRARY_FILE} ${ZLIB_LIBRARY_FILE} ${LIBPNG_LIBRARY_FILE})

target_compile_definitions(enhanced_cloudscapes PRIVATE "XPLM200=1" "XPLM210=1" "XPLM300=1" "XPLM301=1" "XPLM302=1" "XPLM303=1" "XPLM_DEPRECATED=1")

set_target_properties(enhanced_cloudscapes PROPERTIES PREFIX "")
set_target_properties(enhanced_cloudscapes PROPERTIES SUFFIX ".xpl")

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	target_compile_definitions(enhanced_cloudscapes PRIVATE "IBM=1" "GLEW_BUILD=1" "NOMINMAX=1")

	set_target_properties(enhanced_cloudscapes PROPERTIES OUTPUT_NAME "win")
	
	install(DIRECTORY "Enhanced_Cloudscapes" DESTINATION ${CMAKE_INSTALL_PREFIX})
	install(TARGETS enhanced_cloudscapes RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/Enhanced_Cloudscapes")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	target_compile_definitions(enhanced_cloudscapes PRIVATE "LIN=1")

	set_target_properties(enhanced_cloudscapes PROPERTIES OUTPUT_NAME "lin")

	install(DIRECTORY "Enhanced_Cloudscapes" DESTINATION ${CMAKE_INSTALL_PREFIX})
	install(TARGETS enhanced_cloudscapes LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}/Enhanced_Cloudscapes")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	target_compile_definitions(enhanced_cloudscapes PRIVATE "APL=1")

	set_target_properties(enhanced_cloudscapes PROPERTIES OUTPUT_NAME "mac")

	install(DIRECTORY "Enhanced_Cloudscapes" DESTINATION ${CMAKE_INSTALL_PREFIX})
	install(TARGETS enhanced_cloudscapes FRAMEWORK DESTINATION "${CMAKE_INSTALL_PREFIX}/Enhanced_Cloudscapes")
endif()