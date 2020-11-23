#ifdef IBM
#include <Windows.h>
#endif

#include <GL/glew.h>

#include <XPLMDataAccess.h>
#include <XPLMDisplay.h>

#include <simulator_objects.hpp>
#include <plugin_objects.hpp>

#include <rendering_program.hpp>
#include <post_processing_program.hpp>

#include <cstring>

#ifdef IBM
BOOL APIENTRY DllMain(IN HINSTANCE dll_handle, IN DWORD call_reason, IN LPVOID reserved)
{
	return TRUE;
}
#endif

int draw_callback(XPLMDrawingPhase drawing_phase, int is_before, void* callback_reference)
{
	simulator_objects::update();
	plugin_objects::update();

	rendering_program::call();
	post_processing_program::call();

	return 1;
}

PLUGIN_API int XPluginStart(char* plugin_name, char* plugin_signature, char* plugin_description)
{
	std::strcpy(plugin_name, "Enhanced Cloudscapes");
	std::strcpy(plugin_signature, "FarukEroglu2048.enhanced_cloudscapes");
	std::strcpy(plugin_description, "Volumetric Clouds for X-Plane 11");

	glewInit();

	simulator_objects::initialize();
	plugin_objects::initialize();

	rendering_program::initialize();
	post_processing_program::initialize();

	XPLMRegisterDrawCallback(draw_callback, xplm_Phase_Modern3D, 0, nullptr);

	return 1;
}

PLUGIN_API void XPluginStop(void)
{
	
}

PLUGIN_API int XPluginEnable(void)
{
	return 1;
}

PLUGIN_API void XPluginDisable(void)
{

}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID sender_plugin, int message_type, void* callback_parameters)
{

}