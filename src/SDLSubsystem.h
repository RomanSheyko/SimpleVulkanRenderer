#ifndef _SDLSUBSYSTEM_H
#define _SDLSUBSYSTEM_H
#include "WindowSubsystem.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include "WindowSubsystemException.h"
#include "VulkanRenderer.h"
#include "SceneObject.h"

class SDLSubsystem :
    public WindowSubsystem
{
public:
    SDLSubsystem(const char* window_name, size_t width, size_t height);
    void mainLoop() override;
    ~SDLSubsystem() override;
private:
    SDL_Window* window;
    std::unique_ptr<VulkanRenderer> renderer;
    std::vector<SceneObject> sceneObjects;
    std::vector<const char*> requiredInstanceExtentions;
    void createSurface() override;
    void createWindow(const char* window_name, size_t width, size_t height) override;
    void loadSceneObjects();
};

#endif
