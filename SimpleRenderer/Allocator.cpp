#include "Allocator.h"
#include <cstdlib>

void* Allocator::allocation(
	size_t                  size,
	size_t                  alignment,
	VkSystemAllocationScope allocationScope)
{
	//TODO: write allocation
	return _aligned_malloc(size, alignment);
}

void* Allocator::reallocation(
	void* pOriginal,
	size_t                  size,
	size_t                  alignment,
	VkSystemAllocationScope allocationScope)
{
	//TODO: write reallocation
	return _aligned_realloc(pOriginal, size, alignment);
}

void Allocator::free(void* pMemory)
{
	//TODO: write memory free
	_aligned_free(pMemory);
}

void* VKAPI_CALL Allocator::Allocation(
	void*                   pUserData,
	size_t                  size,
	size_t                  alignment,
	VkSystemAllocationScope allocationScope) 
{
	return static_cast<Allocator*>(pUserData)->allocation(size, alignment, allocationScope);
}

void* VKAPI_CALL Allocator::Reallocation(
	void*                   pUserData,
	void*                   pOriginal,
	size_t                  size,
	size_t                  alignment,
	VkSystemAllocationScope allocationScope)
{
	return static_cast<Allocator*>(pUserData)->reallocation(pOriginal, size, alignment, allocationScope);
}

void VKAPI_CALL Allocator::Free(
	void*                  pUserData,
	void*                  pMemory)
{
	return static_cast<Allocator*>(pUserData)->free(pMemory);
}