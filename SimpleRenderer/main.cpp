#include <iostream>
#include <vulkan/vulkan.h>
//#include <SDL2/SDL.h>
//#include <SDL2/SDL_vulkan.h>
#include <vector>
#include "VulkanRenderer.h"
#define RENDERER VulkanRenderer

int main(int argc, char* argv[]) {
	try {
		RENDERER renderer;
	}
	catch (RendererException & e) {
		std::cout << e.what() << std::endl;
	}

	system("pause");
	return EXIT_SUCCESS;
}