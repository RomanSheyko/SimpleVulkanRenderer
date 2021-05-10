#ifndef Model_h
#define Model_h
#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vector>
#include "VulkanRenderer.h"

class Model
{
public:
    
    struct Vertex {
        glm::vec2 positon;
        
        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };
    
    Model(VulkanRenderer& renderer, const std::vector<Vertex> &vertices);
    ~Model();
    
    Model(const Model&) = delete;
    Model &operator=(const Model&) = delete;
    
    void bind(VkCommandBuffer commandBuffer);
    void draw(VkCommandBuffer commandBuffer);
private:
    uint32_t vertexCount;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VulkanRenderer& renderer;
    
    void createVertexBuffers(const std::vector<Vertex> &vertices);
};

#endif /* Model_h */
