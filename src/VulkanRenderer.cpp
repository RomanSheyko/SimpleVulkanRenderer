#include "VulkanRenderer.h"
#include <iostream>
#include <array>
#include "SceneObject.h"

VulkanRenderer::VulkanRenderer(std::vector<const char*>& requiredInstanceExtentions, VkAllocationCallbacks* allocator) :
render_pass(VK_NULL_HANDLE),
pipelineLayout(VK_NULL_HANDLE),
descriptorSetLayout(VK_NULL_HANDLE),
descriptorPool(VK_NULL_HANDLE),
command_pool(VK_NULL_HANDLE),
command_buffer(VK_NULL_HANDLE),
render_complete_semaphore(VK_NULL_HANDLE)
{
    surface.surface = VK_NULL_HANDLE;
    gpu.gpu = VK_NULL_HANDLE;
    swapchain.swapchain = VK_NULL_HANDLE;
    depth_stencil.depth_stecil_image = VK_NULL_HANDLE;
    depth_stencil.depth_stecil_image_view = VK_NULL_HANDLE;
    depth_stencil.depth_stencil_format = VK_FORMAT_UNDEFINED;
    depth_stencil.stencil_available = false;
    depth_stencil.depth_stencil_image_memory = VK_NULL_HANDLE;
    swapchain.active_swapchain_image_id = UINT32_MAX;
    swapchain.swapchain_image_available = VK_NULL_HANDLE;
    swapchain.swapchain_image_count = SWAPCHAIN_BUFFER_COUNT;
	this->allocator = allocator;
	initInstance(requiredInstanceExtentions);
#ifdef DEBUG_APPLICATION
	VkDebugReportCallbackCreateInfoEXT debugReportCallbackcreateInfo = {};
	debugReportCallbackcreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	debugReportCallbackcreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	debugReportCallbackcreateInfo.pfnCallback = vulkanDebugCallback;

	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT =
		(PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");

	if (vkCreateDebugReportCallbackEXT(instance, &debugReportCallbackcreateInfo, allocator, &(this->reportCallback)) != VK_SUCCESS) {
		throw RendererException("\"vkCreateDebugReportCallbackEXT\" error");
	}
	std::cout << "Vulkan: report callback is set " << std::endl;
#endif
	createLogicalDevice();
    pipeline = nullptr;
	std::cout << "Vulkan: vulkan successfully initialized" << std::endl;
}

VulkanRenderer::~VulkanRenderer() {
    vkQueueWaitIdle(queue.queue);
    if(pipeline != nullptr) delete pipeline;
    
    if(descriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(logical_device, descriptorSetLayout, nullptr);
    }
    
    if(pipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(logical_device, pipelineLayout, nullptr);
    }
    
    if(descriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(logical_device, descriptorPool, nullptr);
    }
    
    if(swapchain.swapchain_image_available != VK_NULL_HANDLE)
    {
        vkDestroyFence(logical_device, swapchain.swapchain_image_available, allocator);
    }
    
    if(render_complete_semaphore != VK_NULL_HANDLE)
    {
        vkDestroySemaphore(logical_device, render_complete_semaphore, nullptr);
    }
    
    if(command_pool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(logical_device, command_pool, nullptr);
    }
    
    for(auto& el : swapchain.framebuffers)
    {
        vkDestroyFramebuffer(logical_device, el, allocator);
    }
    
    if(render_pass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(logical_device, render_pass, allocator);
    }
    
    if(depth_stencil.depth_stecil_image_view != VK_NULL_HANDLE)
    {
        vkDestroyImageView(logical_device, depth_stencil.depth_stecil_image_view, allocator);
    }
    
    vkFreeMemory(logical_device, depth_stencil.depth_stencil_image_memory, allocator);
    
    if(depth_stencil.depth_stecil_image != VK_NULL_HANDLE)
    {
        vkDestroyImage(logical_device, depth_stencil.depth_stecil_image, allocator);
    }
    
    for(auto& el : swapchain.swapchain_image_views)
    {
        vkDestroyImageView(logical_device, el, allocator); 
    }
    
    if(swapchain.swapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(logical_device, swapchain.swapchain, allocator);
    }
	if (surface.surface != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR(instance, surface.surface, allocator);
	}
	//destroying logical device 
	vkDeviceWaitIdle(logical_device);
	vkDestroyDevice(logical_device, allocator);
#ifdef DEBUG_APPLICATION
    PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT =
        (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
    vkDestroyDebugReportCallbackEXT(instance, this->reportCallback, allocator);
#endif
	//destroying instance
	vkDestroyInstance(instance, allocator);
}

void VulkanRenderer::initInstance(std::vector<const char*>& requiredInstanceExtentions) {
	VkInstanceCreateInfo info = {};
	VkApplicationInfo app_info = {};

	/*-----------------------Required features section------------------------*/
	gpu.requiredDeviceFeatures = {};
    gpu.requiredDeviceFeatures.multiDrawIndirect = VK_TRUE;
    gpu.requiredDeviceFeatures.tessellationShader = VK_TRUE;
	//requiredDeviceFeatures.geometryShader = VK_TRUE;
	/*------------------------------------------------------------------------*/

	//std::vector<const char*> requiredInstanceExtentions = { RQUIRED_INSTANCE_EXTENTIONS };

	//filling out application info
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pNext = nullptr;
	app_info.pApplicationName = APP_NAME;
	app_info.applicationVersion = APP_VERSION;
	app_info.apiVersion = VULKAN_VERSION;

	//matching with list of rquired instance extensions
	if (!requiredInstanceExtentions.empty()) {
		uint32_t numInstaceExtention = 0;
		std::vector<VkExtensionProperties> instanceSupportedExtentions;
		vkEnumerateInstanceExtensionProperties(nullptr, &numInstaceExtention, nullptr);
		if (numInstaceExtention > 0) {
			instanceSupportedExtentions.resize(numInstaceExtention);
			vkEnumerateInstanceExtensionProperties(nullptr, &numInstaceExtention, instanceSupportedExtentions.data());
			bool found;
			for (auto extention_name : requiredInstanceExtentions) {
				found = false;
				for (auto& supportedExtention : instanceSupportedExtentions) {
					if (strcmp(extention_name, supportedExtention.extensionName) == 0) {
						found = true;
						break;
					}
				}
				if (!found) throw RendererException("one or more of requested instance extentions are not supported");
			}
		}
		else if (requiredInstanceExtentions.size() > 0) {
			throw RendererException("one or more of requested instance extentions are not supported");
		}
	}

	std::vector<const char*> requiredInstanceLayers = { REQUIRED_INSTANCE_VALIDATION_LAYERS };
	if (!requiredInstanceLayers.empty()) {
		uint32_t numInstanceLayers = 0;
		std::vector<VkLayerProperties> supportedInstanceLayers;
		vkEnumerateInstanceLayerProperties(&numInstanceLayers, nullptr);
		if (numInstanceLayers != 0) {
			supportedInstanceLayers.resize(numInstanceLayers);
			vkEnumerateInstanceLayerProperties(&numInstanceLayers, supportedInstanceLayers.data());
			bool found;
			for (auto layer_name : requiredInstanceLayers) {
				found = false;
				for (auto& supportedLayer : supportedInstanceLayers) {
					if (strcmp(layer_name, supportedLayer.layerName) == 0) {
						found = true;
						break;
					}
				}
				if (!found) throw RendererException("one or more of requested instance layers are not supported");
			}
		}
		else if (requiredInstanceLayers.size() > 0) {
			throw RendererException("one or more of requested instance layers are not supported");
		}
	}

	//filling out instace info
	info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	info.pApplicationInfo = &app_info;
	if (!requiredInstanceExtentions.empty()) {
		info.ppEnabledExtensionNames = requiredInstanceExtentions.data();
		info.enabledExtensionCount = (uint32_t)requiredInstanceExtentions.size();
	}
	if (!requiredInstanceLayers.empty()) {
		info.ppEnabledLayerNames = requiredInstanceLayers.data();
		info.enabledLayerCount = (uint32_t)requiredInstanceLayers.size();
	}

	if (vkCreateInstance(&info, allocator, &instance) != VK_SUCCESS)
		throw RendererException("unable to create instance");
}

void VulkanRenderer::selectPhysicalDevice() {
	uint32_t physicalDeviceCount = 0;

	if (vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr) == VK_SUCCESS) {
        gpu.number_of_selected_device = PHYSICAL_DEVICE_NUM;
		setPhysicalDevice(PHYSICAL_DEVICE_NUM, physicalDeviceCount);
	}
	else throw RendererException("\"vkEnumeratePhysicalDevices\" error");
}

void VulkanRenderer::setPhysicalDevice(uint32_t selected_device_num, uint32_t physicalDevicesCount) {
    gpu.physicalDevices.resize(physicalDevicesCount);

	if (vkEnumeratePhysicalDevices(instance, &physicalDevicesCount, &gpu.physicalDevices[0]) == VK_SUCCESS) {
		vkGetPhysicalDeviceProperties(gpu.physicalDevices[selected_device_num], &gpu.selectedDeviceProperties);
		vkGetPhysicalDeviceMemoryProperties(gpu.physicalDevices[selected_device_num], &gpu.selectedDeviceMemoryProperties);
		setQueues();
	}
	else throw RendererException("\"vkEnumeratePhysicalDevices\" error");
    std::cout << "GPU: " << gpu.selectedDeviceProperties.deviceName << std::endl;
    gpu.gpu = gpu.physicalDevices[selected_device_num];
}

void VulkanRenderer::checkFeatures() {
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(gpu.physicalDevices[gpu.number_of_selected_device], &supportedFeatures);

	uint32_t offset = 0;
	while (offset < sizeof(VkPhysicalDeviceFeatures)) {
		if (*((VkBool32*)((uint8_t*)&gpu.requiredDeviceFeatures + offset)) == VK_TRUE) {
			if (*((VkBool32*)((uint8_t*)&supportedFeatures + offset)) != VK_TRUE)
				throw RendererException("required feature is not supported");
		}
		offset += sizeof(VkBool32);
	}
}

void VulkanRenderer::setQueues() {
	queue.queueFamilyPropertyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(gpu.physicalDevices[gpu.number_of_selected_device], &queue.queueFamilyPropertyCount, nullptr);
	if (queue.queueFamilyPropertyCount == 0)
		throw RendererException("zero queues detected");
    queue.queueFamilyProperties.resize(queue.queueFamilyPropertyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(gpu.physicalDevices[gpu.number_of_selected_device], &queue.queueFamilyPropertyCount, queue.queueFamilyProperties.data());
	for (uint32_t i = 0; i < queue.queueFamilyProperties.size(); i++)
	{
		if ((queue.queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queue.queueFamilyProperties[i].queueCount >= QUEUE_COUNT))
		{
            queue.graphics_famaly_index = i;
			return;
		}
	}
	throw RendererException("no graphics-enabled queues detected");
}

void VulkanRenderer::createLogicalDevice() {
	selectPhysicalDevice();
	checkFeatures();
	std::vector<float> priorities = {0};
	const  VkDeviceQueueCreateInfo deviceQueueCreateInfo = {
		VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, //sType
		nullptr,                                    //pNext
		0,                                          //flags
        queue.graphics_famaly_index,                //queueFamilyIndex
		QUEUE_COUNT,                                //queueCount
		priorities.data()                           //pQueuePriorities
	};
	std::vector<const char*> requiredDeviceExtensions = { REQUIRED_DEVICE_EXTENTIONS };
	//matching with list of rquired device extensions
	if (!requiredDeviceExtensions.empty()) {
		uint32_t numDeviceExtention = 0;
		std::vector<VkExtensionProperties> deviceSupportedExtentions;
		vkEnumerateDeviceExtensionProperties(gpu.physicalDevices[gpu.number_of_selected_device], nullptr, &numDeviceExtention, nullptr);
		if (numDeviceExtention > 0) {
			deviceSupportedExtentions.resize(numDeviceExtention);
			vkEnumerateDeviceExtensionProperties(gpu.physicalDevices[gpu.number_of_selected_device], nullptr, &numDeviceExtention, deviceSupportedExtentions.data());
			bool found;
			for (auto extention_name : requiredDeviceExtensions) {
				found = false;
				for (auto& supportedExtention : deviceSupportedExtentions) {
					if (strcmp(extention_name, supportedExtention.extensionName) == 0) {
						found = true;
						break;
					}
				}
				if (!found) throw RendererException("one or more of requested device extentions are not supported");
			}
		}
		else if (requiredDeviceExtensions.size() > 0) {
			throw RendererException("one or more of requested device extentions are not supported");
		}
	}
	std::vector<const char*> requiredDeviceLayers = { REQUIRED_DEVICE_VALIDATION_LAYERS };
	if (!requiredDeviceLayers.empty()) {
		uint32_t numDeviceLayers = 0;
		std::vector<VkLayerProperties> supportedDeviceLayers;
		vkEnumerateDeviceLayerProperties(gpu.physicalDevices[gpu.number_of_selected_device], &numDeviceLayers, nullptr);
		if (numDeviceLayers != 0) {
			supportedDeviceLayers.resize(numDeviceLayers);
			vkEnumerateDeviceLayerProperties(gpu.physicalDevices[gpu.number_of_selected_device], &numDeviceLayers, supportedDeviceLayers.data());
			bool found;
			for (auto layer_name : requiredDeviceLayers) {
				found = false;
				for (auto& supportedLayer : supportedDeviceLayers) {
					if (strcmp(layer_name, supportedLayer.layerName) == 0) {
						found = true;
						break;
					}
				}
				if (!found) throw RendererException("one or more of requested device layers are not supported");
			}
		}
		else if (requiredDeviceLayers.size() > 0) {
			throw RendererException("one or more of requested device layers are not supported");
		}
	}
	const VkDeviceCreateInfo deviceCreateInfo = {
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,      //sType
		nullptr,                                   //pNext
		0,                                         //flags
		1,                                         //queueCreateInfoCount
		&deviceQueueCreateInfo,                    //pQueueCreateInfos
		(uint32_t)requiredDeviceLayers.size(),     //enabledLayerCount
		requiredDeviceLayers.data(),               //ppEnabledLayerNames
		(uint32_t)requiredDeviceExtensions.size(), //enabledExtentionCount
		requiredDeviceExtensions.data(),           //ppEnabledExtensionNames
		&gpu.requiredDeviceFeatures                //pEnabledFeatures
	};

	if (vkCreateDevice(gpu.physicalDevices[gpu.number_of_selected_device], &deviceCreateInfo, allocator, &logical_device) != VK_SUCCESS) {
		throw RendererException("unable to create logical device");
	}
    vkGetDeviceQueue(logical_device, queue.graphics_famaly_index, 0, &queue.queue);
}

void VulkanRenderer::getSurfaceCapabilities()
{
    VkBool32 WSI_supported = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(gpu.gpu, queue.graphics_famaly_index, surface.surface, &WSI_supported);
    if(!WSI_supported) throw RendererException("WSI not supported");
    
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu.gpu, surface.surface, &surface.surface_capabilities);
    if(surface.surface_capabilities.currentExtent.width < UINT32_MAX)
    {
        surface.surface_size_x = surface.surface_capabilities.currentExtent.width;
    }
    
    if(surface.surface_capabilities.currentExtent.height < UINT32_MAX)
    {
        surface.surface_size_y = surface.surface_capabilities.currentExtent.height;
    }
    
    uint32_t format_count = 0;
    
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.gpu, surface.surface, & format_count, nullptr);
    
    if(format_count < 1) throw RendererException("surface formats missing");
    
    std::vector<VkSurfaceFormatKHR> formats(format_count);
    
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.gpu, surface.surface, & format_count, formats.data());
    
    if(formats[0].format == VK_FORMAT_UNDEFINED)
    {
        surface.surface_format.format = VK_FORMAT_B8G8R8_UNORM;
        surface.surface_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    }
    else {
        surface.surface_format = formats[0];
    }
}

void VulkanRenderer::createSwapchain() {
    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    
    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu.gpu, surface.surface, &present_mode_count, nullptr);
    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu.gpu, surface.surface, &present_mode_count, present_modes.data());
    for(auto& el : present_modes)
    {
        if(el == VK_PRESENT_MODE_MAILBOX_KHR) present_mode = el;
    }
    
    VkSwapchainCreateInfoKHR swapchain_create_info {};
    swapchain_create_info.sType                  = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.surface                = surface.surface;
    swapchain_create_info.minImageCount          = SWAPCHAIN_BUFFER_COUNT;
    swapchain_create_info.imageFormat            = surface.surface_format.format;
    swapchain_create_info.imageColorSpace        = surface.surface_format.colorSpace;
    swapchain_create_info.imageExtent.width      = surface.surface_size_x;
    swapchain_create_info.imageExtent.height     = surface.surface_size_y;
    swapchain_create_info.imageArrayLayers       = 1;
    swapchain_create_info.imageUsage             = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode       = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.queueFamilyIndexCount  = 0;
    swapchain_create_info.pQueueFamilyIndices    = nullptr;
    swapchain_create_info.preTransform           = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha         = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode            = present_mode;
    swapchain_create_info.clipped                = VK_TRUE;
    swapchain_create_info.oldSwapchain           = nullptr;
    
    swapchain.extent = swapchain_create_info.imageExtent;
    
    vkCreateSwapchainKHR(logical_device, &swapchain_create_info, allocator, &swapchain.swapchain);
    
    vkGetSwapchainImagesKHR(logical_device, swapchain.swapchain, &swapchain.swapchain_image_count, nullptr);
}

void VulkanRenderer::createSwapchainImages() { 
    swapchain.swapchain_images.resize(swapchain.swapchain_image_count);
    swapchain.swapchain_image_views.resize(swapchain.swapchain_image_count);
    
    vkGetSwapchainImagesKHR(logical_device, swapchain.swapchain, &swapchain.swapchain_image_count, swapchain.swapchain_images.data());
    
    for(uint32_t i = 0; i < swapchain.swapchain_image_count; i++)
    {
        VkImageViewCreateInfo image_view_create_info {};
        image_view_create_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image                           = swapchain.swapchain_images[i];
        image_view_create_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format                          = surface.surface_format.format;
        image_view_create_info.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.baseMipLevel   = 0;
        image_view_create_info.subresourceRange.levelCount     = 1;
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.layerCount     = 1;
        
        vkCreateImageView(logical_device, &image_view_create_info, allocator, &swapchain.swapchain_image_views[i]);
    }
}

void VulkanRenderer::createDepthStecilImage()
{
    std::vector<VkFormat> try_formats {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D16_UNORM};
    
    for(auto& el : try_formats)
    {
        VkFormatProperties format_properties{};
        vkGetPhysicalDeviceFormatProperties(gpu.gpu, el, &format_properties);
        
        if (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            depth_stencil.depth_stencil_format = el;
            break;
        }
    }
    
    if(depth_stencil.depth_stencil_format == VK_FORMAT_UNDEFINED)
    {
        throw RendererException("Depth stecil format not selected");
    }
    
    if((depth_stencil.depth_stencil_format == VK_FORMAT_D32_SFLOAT_S8_UINT) ||
       (depth_stencil.depth_stencil_format == VK_FORMAT_D24_UNORM_S8_UINT) ||
       (depth_stencil.depth_stencil_format == VK_FORMAT_D16_UNORM_S8_UINT) ||
       (depth_stencil.depth_stencil_format == VK_FORMAT_D32_SFLOAT) ||
       (depth_stencil.depth_stencil_format == VK_FORMAT_D16_UNORM))
    {
        depth_stencil.stencil_available = true;
    }
    
    VkImageCreateInfo image_create_info {};
    image_create_info.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.flags                 = 0;
    image_create_info.imageType             = VK_IMAGE_TYPE_2D;
    image_create_info.format                = depth_stencil.depth_stencil_format;
    image_create_info.extent.width          = surface.surface_size_x;
    image_create_info.extent.height         = surface.surface_size_y;
    image_create_info.extent.depth          = 1;
    image_create_info.mipLevels             = 1;
    image_create_info.arrayLayers           = 1;
    image_create_info.samples               = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling                = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage                 = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    image_create_info.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.queueFamilyIndexCount = VK_QUEUE_FAMILY_IGNORED;
    image_create_info.pQueueFamilyIndices   = nullptr;
    image_create_info.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;
    
    vkCreateImage(logical_device, &image_create_info, allocator, &depth_stencil.depth_stecil_image);
    
    VkMemoryRequirements image_memory_requirements {};
    vkGetImageMemoryRequirements(logical_device, depth_stencil.depth_stecil_image, &image_memory_requirements);
    auto required_property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    
    uint32_t memory_index = UINT32_MAX;
    for(uint32_t i = 0; i < gpu.selectedDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if(image_memory_requirements.memoryTypeBits & (1 << i))
        {
            if((gpu.selectedDeviceMemoryProperties.memoryTypes[i].propertyFlags & required_property) == required_property)
            {
                memory_index = i;
                break;
            }
        }
    }
    
    if(memory_index == UINT32_MAX) throw RendererException("memory index not selected");
    
    VkMemoryAllocateInfo memory_allocate_info {};
    memory_allocate_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize  = image_memory_requirements.size;
    memory_allocate_info.memoryTypeIndex = memory_index;
    
    vkAllocateMemory(logical_device, &memory_allocate_info, allocator, &depth_stencil.depth_stencil_image_memory);
    vkBindImageMemory(logical_device, depth_stencil.depth_stecil_image, depth_stencil.depth_stencil_image_memory, 0);
    
    VkImageViewCreateInfo image_view_create_info {};
    image_view_create_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.image                           = depth_stencil.depth_stecil_image;
    image_view_create_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format                          = depth_stencil.depth_stencil_format;
    image_view_create_info.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT | (depth_stencil.stencil_available ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
    image_view_create_info.subresourceRange.baseMipLevel   = 0;
    image_view_create_info.subresourceRange.levelCount     = 1;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.layerCount     = 1;
    
    vkCreateImageView(logical_device, &image_view_create_info, allocator, &depth_stencil.depth_stecil_image_view);
}

void VulkanRenderer::createRenderPass()
{
    std::array<VkAttachmentDescription, 2> attachments {};
    attachments[0].flags          = 0;
    attachments[0].format         = depth_stencil.depth_stencil_format;
    attachments[0].samples        = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    attachments[1].flags          = 0;
    attachments[1].format         = surface.surface_format.format;
    attachments[1].samples        = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    VkAttachmentReference subpass_0_depth_stencil_attachment {};
    subpass_0_depth_stencil_attachment.attachment = 0;
    subpass_0_depth_stencil_attachment.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    std::array<VkAttachmentReference, 1> subpass_0_color_attachments {};
    subpass_0_color_attachments[0].attachment = 1;
    subpass_0_color_attachments[0].layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    std::array<VkSubpassDescription, 1> subpasses {};
    subpasses[0].pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[0].inputAttachmentCount    = 0;
    subpasses[0].pInputAttachments       = nullptr;
    subpasses[0].colorAttachmentCount    = subpass_0_color_attachments.size();
    subpasses[0].pColorAttachments       = subpass_0_color_attachments.data();
    subpasses[0].pDepthStencilAttachment = &subpass_0_depth_stencil_attachment;
    
    VkRenderPassCreateInfo render_pass_create_info {};
    render_pass_create_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.attachmentCount = attachments.size();
    render_pass_create_info.pAttachments    = attachments.data();
    render_pass_create_info.subpassCount    = subpasses.size();
    render_pass_create_info.pSubpasses      = subpasses.data();
    render_pass_create_info.dependencyCount = 0;
    render_pass_create_info.pDependencies   = nullptr;
    vkCreateRenderPass(logical_device, &render_pass_create_info, allocator, &render_pass);
}

void VulkanRenderer::createFramebuffers()
{
    swapchain.framebuffers.resize(swapchain.swapchain_image_count);
    for(uint32_t i = 0; i < swapchain.swapchain_image_count; i++)
    {
        std::array<VkImageView, 2> attachments {};
        attachments[0] = depth_stencil.depth_stecil_image_view;
        attachments[1] = swapchain.swapchain_image_views[i];
        
        VkFramebufferCreateInfo framebuffer_create_info {};
        framebuffer_create_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.renderPass      = render_pass;
        framebuffer_create_info.attachmentCount = attachments.size();
        framebuffer_create_info.pAttachments    = attachments.data();
        framebuffer_create_info.width           = surface.surface_size_x;
        framebuffer_create_info.height          = surface.surface_size_y;
        framebuffer_create_info.layers          = 1;
        
        vkCreateFramebuffer(logical_device, &framebuffer_create_info, allocator, &swapchain.framebuffers[i]);
    }
}

void VulkanRenderer::beginRender() { 
    vkAcquireNextImageKHR(logical_device, swapchain.swapchain, UINT64_MAX, VK_NULL_HANDLE, swapchain.swapchain_image_available, &swapchain.active_swapchain_image_id);
    vkWaitForFences(logical_device, 1, &swapchain.swapchain_image_available, VK_TRUE, UINT64_MAX);
    vkResetFences(logical_device, 1, &swapchain.swapchain_image_available);
    vkQueueWaitIdle(queue.queue);
}


void VulkanRenderer::endRender() {
    VkResult present_result = VkResult::VK_RESULT_MAX_ENUM;
    
    VkPresentInfoKHR present_info {};
    present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = semapthores_to_wait.size();
    present_info.pWaitSemaphores    = semapthores_to_wait.data();
    present_info.swapchainCount     = 1;
    present_info.pSwapchains        = &swapchain.swapchain;
    present_info.pImageIndices      = &swapchain.active_swapchain_image_id;
    present_info.pResults           = &present_result;
    
    vkQueuePresentKHR(queue.queue, &present_info);
}

void VulkanRenderer::createSync() {
    VkFenceCreateInfo fence_create_info {};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkCreateFence(logical_device, &fence_create_info, allocator, &swapchain.swapchain_image_available);
}

void VulkanRenderer::init()
{
    getSurfaceCapabilities();
    createSwapchain();
    createSwapchainImages();
    createDepthStecilImage();
    createRenderPass();
    createFramebuffers();
    createCommandPool();
    
    createSemaphores();
    allocateCommandBuffer();
    createSync();
    createDescriptorPool();
    createPipelineLayout();
    createPipeline();
}

void VulkanRenderer::createPipeline()
{
    auto pipelineConfig = Pipeline::defaultPipelineConfigInfo(surface.surface_size_x, surface.surface_size_y);
    pipelineConfig.renderPass = render_pass;
    pipelineConfig.pipelineLayout = pipelineLayout;
    pipeline = new Pipeline(logical_device, std::string(SHADER_PREFIX) + std::string(VERTEX_SHADER), std::string(SHADER_PREFIX) + std::string(FRAGMENT_SHADER), pipelineConfig);
}

void VulkanRenderer::createPipelineLayout() {
    /*
    VkPushConstantRange pushConstantRange {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset     = 0;
    pushConstantRange.size       = sizeof(SimplePushConstantData);
     */
    
    auto layoutBindings = Model::UniformBufferObject::getLayoutBindings();
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = layoutBindings.size();
    layoutInfo.pBindings = layoutBindings.data();
    
    if (vkCreateDescriptorSetLayout(logical_device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw RendererException("failed to create descriptor set layout!");
    }
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
    if(vkCreatePipelineLayout(logical_device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw RendererException("Failed to create pipeline layout");
    }
}

uint32_t VulkanRenderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
  for (uint32_t i = 0; i < gpu.selectedDeviceMemoryProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) &&
        (gpu.selectedDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  throw RendererException("failed to find suitable memory type!");
}

void VulkanRenderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(logical_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to create vertex buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(logical_device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(logical_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate vertex buffer memory!");
    }

    vkBindBufferMemory(logical_device, buffer, bufferMemory, 0);
}

void VulkanRenderer::createDescriptorPool() { 
    VkDescriptorPoolSize poolSize{};
    poolSize.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(swapchain.swapchain_image_count);
    
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes    = &poolSize;
    poolInfo.maxSets       = static_cast<uint32_t>(swapchain.swapchain_image_count);
    
    if (vkCreateDescriptorPool(logical_device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw RendererException("failed to create descriptor pool!");
    }
}

void VulkanRenderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) { 
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = command_pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(logical_device, &allocInfo, &commandBuffer);
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    
    vkEndCommandBuffer(commandBuffer);
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(queue.queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue.queue);
    vkFreeCommandBuffers(logical_device, command_pool, 1, &commandBuffer);
}

void VulkanRenderer::createCommandPool() {
    VkCommandPoolCreateInfo command_pool_create_info {};
    command_pool_create_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.flags            = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_create_info.queueFamilyIndex = queue.graphics_famaly_index;
    
    vkCreateCommandPool(logical_device, &command_pool_create_info, nullptr, &command_pool);
}

void VulkanRenderer::allocateCommandBuffer() { 
    VkCommandBufferAllocateInfo command_buffer_allocate_info {};
    command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool        = command_pool;
    command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = 1;
    
    vkAllocateCommandBuffers(logical_device, &command_buffer_allocate_info, &command_buffer);
}

void VulkanRenderer::createSemaphores() { 
    VkSemaphoreCreateInfo semaphore_create_info {};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    vkCreateSemaphore(logical_device, &semaphore_create_info, nullptr, &render_complete_semaphore);
    semapthores_to_wait.push_back(render_complete_semaphore);
}

void VulkanRenderer::renderSceneObjects(const std::vector<SceneObject>& sceneObjects, const Camera& camInfo)
{
    for(auto& obj : sceneObjects)
    {
        //SimplePushConstantData push {};
        //push.offset = obj.transform2d.translation;
        //push.color = obj.color;
        //push.transform = obj.transform2d.mat2();
        
        //vkCmdPushConstants(commandBuffer, renderer->getPipelineLayput(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
        obj.model->bind(command_buffer);
        obj.model->updateUniformBuffer(swapchain.active_swapchain_image_id, camInfo);
        obj.model->draw(command_buffer);
    }
}

void VulkanRenderer::update(const std::vector<SceneObject>& sceneObjects, const Camera& camInfo) {
    VkCommandBufferBeginInfo command_buffer_begin_info {};
    command_buffer_begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    //command_buffer_begin_info.pInheritanceInfo = nullptr;
    
    vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);
    
    VkRect2D render_area {};
    render_area.offset.x = 0;
    render_area.offset.y = 0;
    render_area.extent   = getSurfaceSize();
    
    std::array<VkClearValue, 2> clear_values {};
    clear_values[0].depthStencil.depth   = 1.0f;
    clear_values[0].depthStencil.stencil = 0.0f;
    clear_values[1].color.float32[0]     = 0.529f;
    clear_values[1].color.float32[1]     = 0.807f;
    clear_values[1].color.float32[2]     = 0.921f;
    clear_values[1].color.float32[3]     = 0.0f;
    
    VkRenderPassBeginInfo render_pass_begin_info {};
    render_pass_begin_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.renderPass      = render_pass;
    render_pass_begin_info.framebuffer     = getActiveFaramebuffer();
    render_pass_begin_info.renderArea      = render_area;
    render_pass_begin_info.clearValueCount = clear_values.size();
    render_pass_begin_info.pClearValues    = clear_values.data();
    
    vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipeline());
    
    renderSceneObjects(sceneObjects, camInfo);
    
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
    
    vkQueueSubmit(queue.queue, 1, &submit_info, VK_NULL_HANDLE);
}

VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char* layerPrefix,
	const char* msg,
	void* userData)
{
	std::cout << "Vulkan debug: " << msg << std::endl;
	return VK_FALSE;
}
