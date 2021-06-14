#ifndef _SDLSUBSYSTEM_H
#define _SDLSUBSYSTEM_H
#include "WindowSubsystem.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include "WindowSubsystemException.h"
#include "VulkanRenderer.h"
#include "SceneObject.h"
#include "Camera.h"
#include "tiny_obj_loader.h"

const std::string MODEL_PATH = "/Users/roman/Downloads/obj/Handgun_obj.obj";

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
    
    size_t width;
    size_t height;
    
    Camera camera;
    
    void createSurface() override;
    void createWindow(const char* window_name, size_t width, size_t height) override;
    void loadSceneObjects();
    void processInput();
};

#endif
