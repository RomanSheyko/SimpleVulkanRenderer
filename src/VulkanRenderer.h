#ifndef VULKAN_RENDERER_H
#define VULKAN_RENDERER_H
#include "RendererException.h"

#define DEBUG_APPLICATION

#ifdef _WIN32
//using windows
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.h>
#include <vector>
#define APP_NAME "My first vulkan renderer"
#define APP_VERSION 1
#define VULKAN_VERSION VK_MAKE_VERSION(1, 0, 0)
/*-----------------------Required instance extentions section----------------------*/
#ifdef _WIN32
#ifdef DEBUG_APPLICATION
#define RQUIRED_INSTANCE_EXTENTIONS\
 VK_KHR_SURFACE_EXTENSION_NAME,\
 VK_KHR_WIN32_SURFACE_EXTENSION_NAME,\
 VK_EXT_DEBUG_REPORT_EXTENSION_NAME
#else
#define RQUIRED_INSTANCE_EXTENTIONS\
 VK_KHR_SURFACE_EXTENSION_NAME,\
 VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#endif
#else
#ifdef DEBUG_APPLICATION
#define RQUIRED_INSTANCE_EXTENTIONS\
 VK_KHR_SURFACE_EXTENSION_NAME,\
 VK_EXT_DEBUG_REPORT_EXTENSION_NAME
#else
#define RQUIRED_INSTANCE_EXTENTIONS\
 VK_KHR_SURFACE_EXTENSION_NAME
#endif
#endif
/*---------------------------------------------------------------------------------*/

/*------------------------Required device extentions section-----------------------*/
#ifdef DEBUG_APPLICATION
#define REQUIRED_DEVICE_EXTENTIONS\
 VK_KHR_SWAPCHAIN_EXTENSION_NAME
#else
#define REQUIRED_DEVICE_EXTENTIONS\
 VK_KHR_SWAPCHAIN_EXTENSION_NAME
#endif
/*---------------------------------------------------------------------------------*/

/*--------------------Required instance validation layers section------------------*/
#ifdef DEBUG_APPLICATION
#define REQUIRED_INSTANCE_VALIDATION_LAYERS\
 "VK_LAYER_KHRONOS_validation"
#else
#define REQUIRED_INSTANCE_VALIDATION_LAYERS
#endif
/*---------------------------------------------------------------------------------*/

/*--------------------Required device validation layers section------------------*/
#ifdef DEBUG_APPLICATION
#define REQUIRED_DEVICE_VALIDATION_LAYERS
#else
#define REQUIRED_DEVICE_VALIDATION_LAYERS
#endif
/*---------------------------------------------------------------------------------*/
#define PHYSICAL_DEVICE_NUM 0 //temporary
#ifdef DEBUG_APPLICATION
VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char* layerPrefix,
	const char* msg,
	void* userData);
#endif

#define SWAPCHAIN_BUFFER_COUNT 2
#define QUEUE_COUNT 1

#include "Allocator.h"

class VulkanRenderer
{
private:
    VkSurfaceKHR surface;
    VkInstance instance;
	VkDevice logical_device;
	uint32_t number_of_selected_device;
	uint32_t queueFamilyPropertyCount;
	VkAllocationCallbacks* allocator;
	uint32_t graphics_famaly_index;
    VkSwapchainKHR swapchain;
    uint32_t swapchain_image_count;
    
    std::vector<VkImage> swapchain_images;
    std::vector<VkImageView> swapchain_image_views;
    
#ifdef DEBUG_APPLICATION
	VkDebugReportCallbackEXT reportCallback;
#endif
	VkPhysicalDeviceFeatures requiredDeviceFeatures;
	std::vector<VkPhysicalDevice> physicalDevices;
    VkPhysicalDevice gpu;
	std::vector<VkQueueFamilyProperties> queueFamilyProperties;
	VkPhysicalDeviceProperties selectedDeviceProperties;
	VkPhysicalDeviceMemoryProperties selectedDeviceMemoryProperties;
    
    VkSurfaceFormatKHR surface_format;
	
	void initInstance(std::vector<const char*>& requiredInstanceExtentions);
	void setPhysicalDevice(uint32_t selected_device_num, uint32_t physicalDevicesCount);
	void selectPhysicalDevice();
	void checkFeatures();
	void setQueues();
    void createLogicalDevice();
    VkSurfaceCapabilitiesKHR surface_capabilities;
    uint32_t surface_size_x;
    uint32_t surface_size_y;
    VkImage depth_stecil_image;
    VkImageView depth_stecil_image_view;
    VkFormat depth_stencil_format;
    bool stencil_available;
    VkRenderPass render_pass;
    VkDeviceMemory depth_stencil_image_memory;
    std::vector<VkFramebuffer> framebuffers;
    
    uint32_t active_swapchain_image_id;
    VkFence swapchain_image_available;
    VkQueue queue;
public:
    const VkDevice& getDevice() const
    {
        return logical_device;
    }
    
    VkInstance& getInstance()
    {
        return instance;
    }
    
    VkSurfaceKHR& getSurface()
    {
        return surface;
    }
    
    uint32_t getGraphicsFamalyIndex()
    {
        return graphics_famaly_index;
    }
    
    VkDevice getDevice()
    {
        return logical_device;
    }
    
    VkAllocationCallbacks* getAllocator()
    {
        return allocator;
    }
    
    VkRenderPass getRenderPass()
    {
        return render_pass;
    }
    
    VkFramebuffer getActiveFaramebuffer()
    {
        return framebuffers[active_swapchain_image_id];
    }
    
    VkExtent2D getSurfaceSize()
    {
        return {surface_size_x, surface_size_y};
    }
    
    VkQueue getQueue()
    {
        return queue;
    }
    
    void getSurfaceCapabilities();
    void createSwapchain();
    void createSwapchainImages();
    void createDepthStecilImage();
    void createRenderPass();
    void createFramebuffers();
    void createSync();
    const VkPhysicalDeviceMemoryProperties& getVulkanPhysicalDeviceMemoryProperties() const;
    
    void beginRender();
    void endRender(std::vector<VkSemaphore>& semapthores_to_wait);
    
	VulkanRenderer(std::vector<const char*>& requiredInstanceExtentions, VkAllocationCallbacks* allocator = nullptr);
	~VulkanRenderer();
};

#endif //VULKAN_RENDERER_H
