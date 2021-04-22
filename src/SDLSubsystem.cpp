#include "SDLSubsystem.h"
#include <stdint.h>
#include <chrono>
#include <thread>

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
	if (!SDL_Vulkan_CreateSurface(window, renderer->getInstance(), &renderer->getSurface()))
	{
		throw WindowSubsystemException("Error creating a surface");
	}
    renderer->getSurfaceCapabilities();
    renderer->createSwapchain();
    renderer->createSwapchainImages();
    renderer->createDepthStecilImage();
    renderer->createRenderPass();
    renderer->createFramebuffers();
}

void SDLSubsystem::mainLoop()
{
	SDL_Event e;
	bool exit = false;
	while (!exit)
	{
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				exit = true;
			}
		}
        //std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

SDLSubsystem::~SDLSubsystem()
{
	SDL_DestroyWindow(window);
	SDL_Quit();
}
