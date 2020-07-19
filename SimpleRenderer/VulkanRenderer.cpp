#include "VulkanRenderer.h"
#include <iostream>

VulkanRenderer::VulkanRenderer(VkAllocationCallbacks* allocator) : surface(VK_NULL_HANDLE)
{
	this->allocator = allocator;
	initInstance();
#ifdef DEBUG_APPLICATION
	VkDebugReportCallbackCreateInfoEXT debugReportCallbackcreateInfo = {};
	debugReportCallbackcreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	debugReportCallbackcreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	debugReportCallbackcreateInfo.pfnCallback = vulkanDebugCallback;

	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT =
		(PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");

	if (vkCreateDebugReportCallbackEXT(instance, &debugReportCallbackcreateInfo, nullptr, &(this->reportCallback)) != VK_SUCCESS) {
		throw RendererException("\"vkCreateDebugReportCallbackEXT\" error");
	}
	std::cout << "Vulkan: report callback is set " << std::endl;
#endif
	createLogicalDevice();
	std::cout << "Vulkan: vulkan successfully initialized" << std::endl;
}

VulkanRenderer::~VulkanRenderer() {
	if (surface != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR(instance, surface, nullptr);
	}
	//destroying logical device 
	vkDeviceWaitIdle(logical_device);
	vkDestroyDevice(logical_device, allocator);
	//destroying instance
	vkDestroyInstance(instance, allocator);
}

void VulkanRenderer::initInstance() {
	VkInstanceCreateInfo info = {};
	VkApplicationInfo app_info = {};

	/*-----------------------Required features section------------------------*/
	requiredDeviceFeatures = {};
	requiredDeviceFeatures.multiDrawIndirect = VK_TRUE;
	requiredDeviceFeatures.tessellationShader = VK_TRUE;
	requiredDeviceFeatures.geometryShader = VK_TRUE;
	/*------------------------------------------------------------------------*/

	std::vector<const char*> requiredInstanceExtentions = { RQUIRED_INSTANCE_EXTENTIONS };

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
		number_of_selected_device = PHYSICAL_DEVICE_NUM;
		setPhysicalDevice(PHYSICAL_DEVICE_NUM, physicalDeviceCount);
	}
	else throw RendererException("\"vkEnumeratePhysicalDevices\" error");
}

void VulkanRenderer::setPhysicalDevice(uint32_t selected_device_num, uint32_t physicalDevicesCount) {
	physicalDevices.resize(physicalDevicesCount);

	if (vkEnumeratePhysicalDevices(instance, &physicalDevicesCount, &physicalDevices[0]) == VK_SUCCESS) {
		vkGetPhysicalDeviceProperties(physicalDevices[selected_device_num], &selectedDeviceProperties);
		vkGetPhysicalDeviceMemoryProperties(physicalDevices[selected_device_num], &selectedDeviceMemoryProperties);
		setQueues();
	}
	else throw RendererException("\"vkEnumeratePhysicalDevices\" error");
}

void VulkanRenderer::checkFeatures() {
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(physicalDevices[number_of_selected_device], &supportedFeatures);

	uint32_t offset = 0;
	while (offset < sizeof(VkPhysicalDeviceFeatures)) {
		if (*((VkBool32*)((uint8_t*)&requiredDeviceFeatures + offset)) == VK_TRUE) {
			if (*((VkBool32*)((uint8_t*)&supportedFeatures + offset)) != VK_TRUE)
				throw RendererException("required feature is not supported");
		}
		offset += sizeof(VkBool32);
	}
}

void VulkanRenderer::setQueues() {
	queueFamilyPropertyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[number_of_selected_device], &queueFamilyPropertyCount, nullptr);
	if (queueFamilyPropertyCount == 0)
		throw RendererException("zero queues detected");
	queueFamilyProperties.resize(queueFamilyPropertyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[number_of_selected_device], &queueFamilyPropertyCount, queueFamilyProperties.data());
	for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
	{
		if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphics_famaly_index = i;
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
		graphics_famaly_index,                      //queueFamilyIndex
		1,                                          //queueCount
		priorities.data()                           //pQueuePriorities
	};
	std::vector<const char*> requiredDeviceExtensions = { REQUIRED_DEVICE_EXTENTIONS };
	//matching with list of rquired device extensions
	if (!requiredDeviceExtensions.empty()) {
		uint32_t numDeviceExtention = 0;
		std::vector<VkExtensionProperties> deviceSupportedExtentions;
		vkEnumerateDeviceExtensionProperties(physicalDevices[number_of_selected_device], nullptr, &numDeviceExtention, nullptr);
		if (numDeviceExtention > 0) {
			deviceSupportedExtentions.resize(numDeviceExtention);
			vkEnumerateDeviceExtensionProperties(physicalDevices[number_of_selected_device], nullptr, &numDeviceExtention, deviceSupportedExtentions.data());
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
		vkEnumerateDeviceLayerProperties(physicalDevices[number_of_selected_device], &numDeviceLayers, nullptr);
		if (numDeviceLayers != 0) {
			supportedDeviceLayers.resize(numDeviceLayers);
			vkEnumerateDeviceLayerProperties(physicalDevices[number_of_selected_device], &numDeviceLayers, supportedDeviceLayers.data());
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
		&requiredDeviceFeatures                    //pEnabledFeatures
	};

	if (vkCreateDevice(physicalDevices[number_of_selected_device], &deviceCreateInfo, allocator, &logical_device) != VK_SUCCESS) {
		throw RendererException("unable to create logical device");
	}
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