#ifndef _ALLOCATOR_H
#define _ALLOCATOR_H
#include <vulkan/vulkan.h>

class Allocator
{
public:
	inline operator VkAllocationCallbacks() const
	{
		VkAllocationCallbacks allocatorInstance;

		allocatorInstance.pUserData = (void*)this;
		allocatorInstance.pfnAllocation = &Allocation;
		allocatorInstance.pfnReallocation = &Reallocation;
		allocatorInstance.pfnFree = &Free;
		allocatorInstance.pfnInternalAllocation = nullptr;
		allocatorInstance.pfnInternalFree = nullptr;

		return allocatorInstance;
	}

private:
	static void* VKAPI_CALL Allocation(
		void*                   pUserData,
		size_t                  size,
		size_t                  alignment,
		VkSystemAllocationScope allocationScope
	);

	static void* VKAPI_CALL Reallocation(
		void*                   pUserData,
		void*                   pOriginal,
		size_t                  size,
		size_t                  alignment,
		VkSystemAllocationScope allocationScope
	);

	static void VKAPI_CALL Free(
		void*                  pUserData,
		void*                  pMemory
	);

	void* allocation(
		size_t                  size,
		size_t                  alignment,
		VkSystemAllocationScope allocationScope
	);

	void* reallocation(
		void*                   pOriginal,
		size_t                  size,
		size_t                  alignment,
		VkSystemAllocationScope allocationScope
	);

	void free(void* pMemory);
};

#endif // !_ALLOCATOR_H
