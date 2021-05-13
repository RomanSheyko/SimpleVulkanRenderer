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
#define SHADER_PREFIX "/Users/roman/Documents/projects/SimpleVulkanRenderer/src/shaders/"
#define VERTEX_SHADER "simple_shader.vert.spv"
#define FRAGMENT_SHADER "simple_shader.frag.spv"

#include "Allocator.h"
#include "Pipeline.h"
#include "Camera.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
class SceneObject;

/*
struct SimplePushConstantData
{
    glm::mat2 transform { 1.f };
    glm::vec2 offset;
    alignas(16) glm::vec3 color;
};
 */

struct PhysicalDevice
{
    VkPhysicalDeviceFeatures requiredDeviceFeatures;
    std::vector<VkPhysicalDevice> physicalDevices;
    VkPhysicalDevice gpu;
    VkPhysicalDeviceProperties selectedDeviceProperties;
    VkPhysicalDeviceMemoryProperties selectedDeviceMemoryProperties;
    uint32_t number_of_selected_device;
};

struct Swapchain
{
    VkSwapchainKHR swapchain;
    std::vector<VkImage> swapchain_images;
    std::vector<VkImageView> swapchain_image_views;
    uint32_t active_swapchain_image_id;
    VkFence swapchain_image_available;
    uint32_t swapchain_image_count;
    std::vector<VkFramebuffer> framebuffers;
    VkExtent2D extent;
};

struct Surface
{
    VkSurfaceKHR surface;
    VkSurfaceFormatKHR surface_format;
    VkSurfaceCapabilitiesKHR surface_capabilities;
    uint32_t surface_size_x;
    uint32_t surface_size_y;
};

struct DepthStencil
{
    VkImage depth_stecil_image;
    VkImageView depth_stecil_image_view;
    VkFormat depth_stencil_format;
    bool stencil_available;
    VkDeviceMemory depth_stencil_image_memory;
};

struct Queue
{
    uint32_t queueFamilyPropertyCount;
    uint32_t graphics_famaly_index;
    std::vector<VkQueueFamilyProperties> queueFamilyProperties;
    VkQueue queue;
};

class VulkanRenderer
{
private:
    VkInstance instance;
	VkDevice logical_device;
	VkAllocationCallbacks* allocator;
    VkRenderPass render_pass;
    
#ifdef DEBUG_APPLICATION
	VkDebugReportCallbackEXT reportCallback;
#endif
    PhysicalDevice gpu;
    Swapchain swapchain;
    Surface surface;
    DepthStencil depth_stencil;
    Queue queue;
    Pipeline* pipeline;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkDescriptorPool descriptorPool;
    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;
    VkSemaphore render_complete_semaphore;
    std::vector<VkSemaphore> semapthores_to_wait;
    
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	
	void initInstance(std::vector<const char*>& requiredInstanceExtentions);
	void setPhysicalDevice(uint32_t selected_device_num, uint32_t physicalDevicesCount);
	void selectPhysicalDevice();
	void checkFeatures();
	void setQueues();
    void createLogicalDevice();
    void getSurfaceCapabilities();
    void createSwapchain();
    void createSwapchainImages();
    void createDepthStecilImage();
    void createRenderPass();
    void createFramebuffers();
    void createSync();
    void createPipeline();
    void createPipelineLayout();
    void createDescriptorPool();
    void createCommandPool();
    void allocateCommandBuffer();
    void createSemaphores();
    void renderSceneObjects(const std::vector<SceneObject>& sceneObjects, const Camera& camInfo);
public:
    PhysicalDevice& getPhysicalDevice()
    {
        return gpu;
    }
    
    Swapchain& getSwapchain()
    {
        return swapchain;
    }
    
    Surface& getSurface()
    {
        return surface;
    }
    
    DepthStencil& getDepthStencil()
    {
        return depth_stencil;
    }
    
    Queue& getQueue()
    {
        return queue;
    }
    
    const VkDevice& getDevice() const
    {
        return logical_device;
    }
    
    VkInstance& getInstance()
    {
        return instance;
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
        return swapchain.framebuffers[swapchain.active_swapchain_image_id];
    }
    
    VkExtent2D getSurfaceSize()
    {
        return {surface.surface_size_x, surface.surface_size_y};
    }
    
    Pipeline& getPipeline()
    {
        return *pipeline;
    }
    
    VkPipelineLayout getPipelineLayout()
    {
        return pipelineLayout;
    }
    
    VkDescriptorSetLayout getDescriptorSetLayout()
    {
        return descriptorSetLayout;
    }
    
    VkDescriptorPool getDescriptorPool()
    {
        return descriptorPool;
    }
    
    VkCommandPool getCommandPool()
    {
        return command_pool;
    }
    
    void init();
    
    void beginRender();
    void endRender();
    
	VulkanRenderer(std::vector<const char*>& requiredInstanceExtentions, VkAllocationCallbacks* allocator = nullptr);
	~VulkanRenderer();
    
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    
    void update(const std::vector<SceneObject>& sceneObjects, const Camera& camInfo);
    void waitIdle()
    {
        vkQueueWaitIdle(queue.queue);
    }
};

#endif //VULKAN_RENDERER_H
