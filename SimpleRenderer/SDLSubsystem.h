#ifndef _SDLSUBSYSTEM_H
#define _SDLSUBSYSTEM_H
#include "WindowSubsystem.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include "WindowSubsystemException.h"

class SDLSubsystem :
    public WindowSubsystem
{
public:
    SDLSubsystem();
    void createWindow(const char* window_name, size_t width, size_t height) override;
    void mainLoop() override;
    ~SDLSubsystem() override;
    void createSurface(VkSurfaceKHR* surface, VkInstance instance) override;
private:
    SDL_Window* window;
};

#endif