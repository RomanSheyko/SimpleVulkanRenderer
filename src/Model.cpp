#include "Model.h"
#include "ModelException.h"

void Model::createVertexBuffers(const std::vector<Vertex> &vertices) { 
    vertexCount = static_cast<uint32_t>(vertices.size());
    if(vertexCount < 3) throw ModelException("Vertex count must be at least 3");
    
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
    
    renderer.createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffer, vertexBufferMemory);
    
    void* data;
    
    vkMapMemory(renderer.getDevice(), vertexBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(renderer.getDevice(), vertexBufferMemory);
}


void Model::draw(VkCommandBuffer commandBuffer) { 
    vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
}


void Model::bind(VkCommandBuffer commandBuffer) { 
    VkBuffer buffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}


Model::~Model() { 
    vkDestroyBuffer(renderer.getDevice(), vertexBuffer, nullptr);
    vkFreeMemory(renderer.getDevice(), vertexBufferMemory, nullptr);
}


Model::Model(VulkanRenderer& renderer, const std::vector<Vertex> &vertices) : renderer(renderer) {
    createVertexBuffers(vertices);
}


std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions() { 
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(1);
    attributeDescriptions[0].binding  = 0;
    attributeDescriptions[0].format   = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].offset   = 0;
    
    return attributeDescriptions;
}


std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions() { 
    std::vector<VkVertexInputBindingDescription> bingingDescriptions(1);
    bingingDescriptions[0].binding   = 0;
    bingingDescriptions[0].stride    = sizeof(Vertex);
    bingingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    return bingingDescriptions;
}