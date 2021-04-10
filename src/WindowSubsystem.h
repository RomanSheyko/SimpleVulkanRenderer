#ifndef _WINDOWSUBSYSTEM_H
#define _WINDOWSUBSYSTEM_H
#include <vulkan/vulkan.h>

class WindowSubsystem
{
protected:
    virtual void createWindow(const char* window_name, size_t width, size_t height) = 0;
    virtual void createSurface() = 0;
public:
	virtual void mainLoop() = 0;
	virtual ~WindowSubsystem() = default;
};

#endif
