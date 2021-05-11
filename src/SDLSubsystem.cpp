#include "SDLSubsystem.h"
#include <stdint.h>
#include <chrono>
#include <thread>
#include <array>
#include <iostream>

SDLSubsystem::SDLSubsystem(const char* window_name, size_t width, size_t height)
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	window = nullptr;
    
    unsigned int count;
    createWindow(window_name, width, height);
    // get count of required extensions
    if(SDL_Vulkan_GetInstanceExtensions(NULL, &count, NULL) == SDL_FALSE)
        throw WindowSubsystemException("Error getting number of required extensions");
    
    static const char *const additionalExtensions[] =
    {
        RQUIRED_INSTANCE_EXTENTIONS // example additional extension
    };
    size_t additionalExtensionsCount = sizeof(additionalExtensions) / sizeof(additionalExtensions[0]);
    size_t extensionCount = count + additionalExtensionsCount;
    const char **names = (const char**)malloc(sizeof(const char *) * extensionCount);
    if(!names)
        throw WindowSubsystemException("Error allocating extansion names array");
    
    // get names of required extensions
    if(!SDL_Vulkan_GetInstanceExtensions(NULL, &count, names))
        throw WindowSubsystemException("Error getting required extensions");
    
    // copy additional extensions after required extensions
    for(size_t i = 0; i < additionalExtensionsCount; i++)
        names[i + count] = additionalExtensions[i];
    
    for(size_t i = 0; i < additionalExtensionsCount + count; i++)
    {
        requiredInstanceExtentions.push_back(names[i]);
    }
    free(names);
    renderer = std::make_unique<VulkanRenderer>(requiredInstanceExtentions);
    
    createSurface();
    loadSceneObjects();
}


void SDLSubsystem::createWindow(const char* window_name, size_t width, size_t height)
{
	window = SDL_CreateWindow(window_name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (int)width, (int)height, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);
	if (window == nullptr)
	{
		throw WindowSubsystemException("Error creating a window");
	}
}

void SDLSubsystem::createSurface()
{
	if (!SDL_Vulkan_CreateSurface(window, renderer->getInstance(), &renderer->getSurface().surface))
	{
		throw WindowSubsystemException("Error creating a surface");
	}
    renderer->init();
}

void SDLSubsystem::mainLoop()
{
	SDL_Event e;
	bool exit = false;
    
    auto timer = std::chrono::steady_clock();
    auto last_time = timer.now();
    uint64_t frame_counter = 0;
    uint64_t fps = 0;
    
	while (!exit)
	{
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				exit = true;
			}
		}
        
        ++frame_counter;
        if(last_time + std::chrono::seconds(1) < timer.now())
        {
            last_time = timer.now();
            fps = frame_counter;
            frame_counter = 0;
            std::cout << "FPS: " << fps << std::endl;
        }
        static int frame = 30;
        frame = (frame + 1) % 100;
        
        renderer->beginRender();
        
        renderer->update(sceneObjects);
        
        renderer->endRender();
	}
    
    renderer->waitIdle();
}

SDLSubsystem::~SDLSubsystem()
{
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void SDLSubsystem::loadSceneObjects() {
    std::vector<Model::Vertex> vertices = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    };
    
    std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0
    };
    
    auto model = std::make_shared<Model>(*renderer.get(), vertices, indices);
    
    auto traingle = SceneObject::createSceneObject();
    traingle.model = model;
    //traingle.color = {.1f, .8f, .1f};
    //traingle.transform2d.translation.x = .2f;
    
    sceneObjects.push_back(std::move(traingle));
}

