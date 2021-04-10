cmake_minimum_required(VERSION 3.5)
project(VulkanRenderer)

find_package(Vulkan REQUIRED)
find_package(SDL2 REQUIRED)

file(GLOB SOURCES "*.cpp")
file(GLOB HEADERS "*.h")

include_directories(VulkanRenderer ${SDL2_INCLUDE_DIRS} ${VULKAN_INCLUDE_DIRS})

add_executable(VulkanRenderer ${SOURCES} ${HEADERS})

target_link_libraries(VulkanRenderer ${SDL2_LIBRARIES} Vulkan::Vulkan)
