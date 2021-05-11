#ifndef Pipeline_h
#define Pipeline_h
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

struct PipelineConfigInfo {
    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    VkPipelineLayout pipelineLayout = nullptr;
    VkRenderPass renderPass = nullptr;
    uint32_t subpass = 0;
};

class Pipeline
{
public:
    Pipeline(VkDevice& device, const std::string& vertFile, const std::string& fragFile, const PipelineConfigInfo& configInfo);
    
    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;
    
    static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t heigth);
    ~Pipeline();
    
    VkPipeline getPipeline()
    {
        return graphicsPipeline;
    }
    
    VkDescriptorSetLayout getDescriptorSetLayout()
    {
        return descriptorSetLayout;
    }
    
private:
    static std::vector<char> readFile(const std::string& filepath);
    
    void createGraphicsPipeline(const std::string& vertFile, const std::string& fragFile, const PipelineConfigInfo& configInfo);
    
    void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
    
    VkPipeline graphicsPipeline;
    VkDevice& device;
    VkDescriptorSetLayout descriptorSetLayout;
    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;
};

#endif /* Pipeline_h */
