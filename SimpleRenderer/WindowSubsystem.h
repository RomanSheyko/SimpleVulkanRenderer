#ifndef _WINDOWSUBSYSTEM_H
#define _WINDOWSUBSYSTEM_H
#include <vulkan/vulkan.h>

class WindowSubsystem
{
public:
	virtual void createWindow(const char* window_name, size_t width, size_t height) = 0;
	virtual void mainLoop() = 0;
	virtual void createSurface(VkSurfaceKHR* surface, VkInstance instance) = 0;
	virtual ~WindowSubsystem() = default;
};

#endif