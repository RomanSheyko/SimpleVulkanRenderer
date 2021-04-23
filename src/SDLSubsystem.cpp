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
    renderer->createSync();
}

void SDLSubsystem::mainLoop()
{
	SDL_Event e;
	bool exit = false;
    VkCommandPool command_pool = VK_NULL_HANDLE;
    VkCommandPoolCreateInfo command_pool_create_info {};
    command_pool_create_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.flags            = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_create_info.queueFamilyIndex = renderer->getGraphicsFamalyIndex();
    
    vkCreateCommandPool(renderer->getDevice(), &command_pool_create_info, renderer->getAllocator(), &command_pool);
    
    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo command_buffer_allocate_info {};
    command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool        = command_pool;
    command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = 1;
    
    vkAllocateCommandBuffers(renderer->getDevice(), &command_buffer_allocate_info, &command_buffer);
    
    VkSemaphore render_complete_semaphore = VK_NULL_HANDLE;
    VkSemaphoreCreateInfo semaphore_create_info {};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    vkCreateSemaphore(renderer->getDevice(), &semaphore_create_info, renderer->getAllocator(), &render_complete_semaphore);
    
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
        
        //std::this_thread::sleep_for(std::chrono::seconds(1));
        renderer->beginRender();
        
        VkCommandBufferBeginInfo command_buffer_begin_info {};
        command_buffer_begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        command_buffer_begin_info.flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        command_buffer_begin_info.pInheritanceInfo = nullptr;
        
        vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);
        
        VkRect2D render_area {};
        render_area.offset.x = 0;
        render_area.offset.y = 0;
        render_area.extent   = renderer->getSurfaceSize();
        
        std::array<VkClearValue, 2> clear_values {};
        clear_values[0].depthStencil.depth   = 0.0f;
        clear_values[0].depthStencil.stencil = 0.0f;
        clear_values[1].color.float32[0]     = 0.0f;
        clear_values[1].color.float32[1]     = 0.0f;
        clear_values[1].color.float32[2]     = 0.0f;
        clear_values[1].color.float32[3]     = 0.0f;
        
        VkRenderPassBeginInfo render_pass_begin_info {};
        render_pass_begin_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.renderPass      = renderer->getRenderPass();
        render_pass_begin_info.framebuffer     = renderer->getActiveFaramebuffer();
        render_pass_begin_info.renderArea      = render_area;
        render_pass_begin_info.clearValueCount = clear_values.size();
        render_pass_begin_info.pClearValues    = clear_values.data();
        
        vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        
        vkCmdEndRenderPass(command_buffer);
        
        vkEndCommandBuffer(command_buffer);
        
        VkSubmitInfo submit_info {};
        submit_info.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.waitSemaphoreCount   = 0;
        submit_info.pWaitSemaphores      = nullptr;
        submit_info.pWaitDstStageMask    = nullptr;
        submit_info.commandBufferCount   = 1;
        submit_info.pCommandBuffers      = &command_buffer;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores    = &render_complete_semaphore;
        
        vkQueueSubmit(renderer->getQueue(), 1, &submit_info, VK_NULL_HANDLE);
        
        std::vector<VkSemaphore> sems_to_wait = {render_complete_semaphore};
        renderer->endRender( sems_to_wait );
	}
    
    vkQueueWaitIdle(renderer->getQueue());
    vkDestroySemaphore(renderer->getDevice(), render_complete_semaphore, renderer->getAllocator());
    vkDestroyCommandPool(renderer->getDevice(), command_pool, renderer->getAllocator());
}

SDLSubsystem::~SDLSubsystem()
{
	SDL_DestroyWindow(window);
	SDL_Quit();
}
