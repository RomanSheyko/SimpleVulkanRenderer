#include <iostream>
#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vector>
#include "VulkanRenderer.h"
#include "Allocator.h"
#include "SDLSubsystem.h"
#define RENDERER VulkanRenderer

int main(int argc, char* argv[]) {
	try {
		SDLSubsystem windowSubsystem("Test", 1440, 960);
		windowSubsystem.mainLoop();
	}
	catch (RendererException & e) {
		std::cout << e.what() << std::endl;
	}
    catch (WindowSubsystemException& e)
    {
        std::cout << e.what() << std::endl;
    }

	return EXIT_SUCCESS;
}
