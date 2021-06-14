#include "Model.h"
#include "ModelException.h"
#include <chrono>

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

void Model::createIndexBuffers(const std::vector<uint32_t> &indices) {
    indexCount = static_cast<uint32_t>(indices.size());
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    renderer.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(renderer.getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t) bufferSize);
    vkUnmapMemory(renderer.getDevice(), stagingBufferMemory);

    renderer.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

    renderer.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(renderer.getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(renderer.getDevice(), stagingBufferMemory, nullptr);
}

void Model::draw(VkCommandBuffer commandBuffer) { 
    //vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
    vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
}

void Model::bind(VkCommandBuffer commandBuffer) { 
    VkBuffer buffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.getPipelineLayout(), 0, 1, &descriptorSets[renderer.getSwapchain().active_swapchain_image_id], 0, nullptr);
}

Model::~Model() {
    vkDestroyBuffer(renderer.getDevice(), indexBuffer, nullptr);
    vkFreeMemory(renderer.getDevice(), indexBufferMemory, nullptr);
    
    vkDestroyBuffer(renderer.getDevice(), vertexBuffer, nullptr);
    vkFreeMemory(renderer.getDevice(), vertexBufferMemory, nullptr);
    
    for (size_t i = 0; i < renderer.getSwapchain().swapchain_image_count; i++) {
        vkDestroyBuffer(renderer.getDevice(), uniformBuffers[i], nullptr);
        vkFreeMemory(renderer.getDevice(), uniformBuffersMemory[i], nullptr);
    }
}


Model::Model(VulkanRenderer& renderer, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices) : renderer(renderer) {
    createVertexBuffers(vertices);
    createIndexBuffers(indices);
    createUniformBuffers();
    createDescriptorSets();
}

void Model::createUniformBuffers() { 
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(renderer.getSwapchain().swapchain_image_count);
    uniformBuffersMemory.resize(renderer.getSwapchain().swapchain_image_count);

    for (size_t i = 0; i < renderer.getSwapchain().swapchain_image_count; i++) {
        renderer.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
    }
}

void Model::updateUniformBuffer(uint32_t currentImage, const Camera& camInfo) {
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    
    UniformBufferObject ubo{};
    //ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.model = glm::mat4(1.0f);
    //ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(camInfo.getPos(), camInfo.getPos() + camInfo.getFront(), camInfo.getUp());
    ubo.proj = glm::perspective(glm::radians(45.0f), renderer.getSurface().surface_size_x / (float) renderer.getSurface().surface_size_y, 0.1f, 100.0f);
    ubo.proj[1][1] *= -1;
    void* data;
    vkMapMemory(renderer.getDevice(), uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(renderer.getDevice(), uniformBuffersMemory[currentImage]);
}

void Model::createDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(renderer.getSwapchain().swapchain_image_count, renderer.getDescriptorSetLayout());
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool     = renderer.getDescriptorPool();
    allocInfo.descriptorSetCount = static_cast<uint32_t>(renderer.getSwapchain().swapchain_image_count);
    allocInfo.pSetLayouts        = layouts.data();
    
    descriptorSets.resize(renderer.getSwapchain().swapchain_image_count);
    if (vkAllocateDescriptorSets(renderer.getDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw RendererException("failed to allocate descriptor sets!");
    }
    
    for (size_t i = 0; i < renderer.getSwapchain().swapchain_image_count; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range  = sizeof(UniformBufferObject);
        
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet           = descriptorSets[i];
        descriptorWrite.dstBinding       = 0;
        descriptorWrite.dstArrayElement  = 0;
        descriptorWrite.descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount  = 1;
        descriptorWrite.pBufferInfo      = &bufferInfo;
        descriptorWrite.pImageInfo       = nullptr; // Optional
        descriptorWrite.pTexelBufferView = nullptr; // Optional
        
        vkUpdateDescriptorSets(renderer.getDevice(), 1, &descriptorWrite, 0, nullptr);
    }
}

std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions() { 
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
    attributeDescriptions[0].binding  = 0;
    attributeDescriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].offset   = offsetof(Vertex, positon);
    
    attributeDescriptions[1].binding  = 0;
    attributeDescriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].offset   = offsetof(Vertex, color);
    
    return attributeDescriptions;
}


std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions() { 
    std::vector<VkVertexInputBindingDescription> bingingDescriptions(1);
    bingingDescriptions[0].binding   = 0;
    bingingDescriptions[0].stride    = sizeof(Vertex);
    bingingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    return bingingDescriptions;
}


std::vector<VkDescriptorSetLayoutBinding> Model::UniformBufferObject::getLayoutBindings() {
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings(1);
    layoutBindings[0].binding            = 0;
    layoutBindings[0].descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBindings[0].descriptorCount    = 1;
    layoutBindings[0].stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
    layoutBindings[0].pImmutableSamplers = nullptr;
    
    return layoutBindings;
}
