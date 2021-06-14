#ifndef Model_h
#define Model_h
#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "VulkanRenderer.h"
#include "Camera.h"

class Model
{
public:
    
    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
        
        static std::vector<VkDescriptorSetLayoutBinding> getLayoutBindings();
    };
    
    struct Vertex {
        glm::vec3 positon;
        glm::vec3 color;
        
        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };
    
    Model(VulkanRenderer& renderer, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);
    ~Model();
    
    Model(const Model&) = delete;
    Model &operator=(const Model&) = delete;
    
    void bind(VkCommandBuffer commandBuffer);
    void draw(VkCommandBuffer commandBuffer);
    void updateUniformBuffer(uint32_t currentImage, const Camera& camInfo);
private:
    uint32_t vertexCount;
    uint32_t indexCount;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    VulkanRenderer& renderer;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<VkDescriptorSet> descriptorSets;
    
    void createVertexBuffers(const std::vector<Vertex> &vertices);
    void createIndexBuffers(const std::vector<uint32_t> &indices);
    void createUniformBuffers();
    void createDescriptorSets();
};

#endif /* Model_h */
