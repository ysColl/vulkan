#ifndef VulkanHeader
#define VulkanHeader
#include <vulkan/vulkan.h>
#endif

#include <vector>
#include <string>
#include <functional>
#include <memory>


namespace PipelineData{
    void DoInit();
    void cleanup();

    class ShaderFactory{
    public:
        static std::unique_ptr<VkPipelineShaderStageCreateInfo[]> getDefaultShaderInfo();
        static std::function<void()> destroyShader;
        static std::vector<char> readFile(std::string);
        static VkShaderModule createShaderModule(const std::vector<char>& code);
    };

    class RenderPassFactory{
        // render pass是一组framebuffer附件，它们定义了渲染的输入和输出。subpass是render pass中的一个阶段，它指定了使用哪些附件来执行渲染命令。
        static void createRenderPass();
    public:
        static VkRenderPass GetRenderPass();
        static void createFramebuffers();
        static std::vector<VkFramebuffer> getSwapChainFramebuffers();
        static void cleanup();
    private:
        static std::vector<VkFramebuffer> swapChainFramebuffers;
        static VkRenderPass renderPass;
    };

    class Pipeline{
    public:
        static void createGraphicsPipeline();
        static void cleanup();
        static VkPipeline getGraphicPipeline();
    private:
        static VkPipelineLayout pipelineLayout;
        static VkPipeline graphicsPipeline;
    };
}