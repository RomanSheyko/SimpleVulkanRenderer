#include "SDLSubsystem.h"

SDLSubsystem::SDLSubsystem()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	window = nullptr;
}


void SDLSubsystem::createWindow(const char* window_name, size_t width, size_t height)
{
	window = SDL_CreateWindow(window_name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (int)width, (int)width, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);
	if (window == nullptr)
	{
		throw WindowSubsystemException("Error creating a window");
	}
}

void SDLSubsystem::createSurface(VkSurfaceKHR* surface, VkInstance instance)
{
	if (!SDL_Vulkan_CreateSurface(window, instance, surface))
	{
		throw WindowSubsystemException("Error creating a surface");
	}
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

	}
}

SDLSubsystem::~SDLSubsystem()
{
	SDL_DestroyWindow(window);
	SDL_Quit();
}