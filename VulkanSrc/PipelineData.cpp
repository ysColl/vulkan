#include "PipelineData.h"
#include "Device.h"
#include "Present.h"
#include "MeshData.h"

#include <fstream>
#include <iterator>
#include <iostream>

namespace PipelineData
{
    void DoInit(){
        Pipeline::createGraphicsPipeline();
        RenderPassFactory::createFramebuffers();
    }
    void cleanup(){
        Pipeline::cleanup();
        RenderPassFactory::cleanup();
    }

    std::function<void()> ShaderFactory::destroyShader;
    std::unique_ptr<VkPipelineShaderStageCreateInfo[]> ShaderFactory::getDefaultShaderInfo()
    {
        // 读取着色器字节码，并使用vkShaderModule包装好
        auto vertShaderCode = readFile("../Shader/vert.spv");
        auto fragShaderCode = readFile("../Shader/frag.spv");

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        // 设置着色器阶段信息，用于传入管线对象中，执行渲染流程
        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        destroyShader = [vertShaderModule, fragShaderModule]()
        {
            vkDestroyShaderModule(Device::VulkanDevice::getLogicalDevice(), fragShaderModule, nullptr);
            vkDestroyShaderModule(Device::VulkanDevice::getLogicalDevice(), vertShaderModule, nullptr);
        };

        return std::unique_ptr<VkPipelineShaderStageCreateInfo[]>{new VkPipelineShaderStageCreateInfo[2]{vertShaderStageInfo, fragShaderStageInfo}};
    }

    VkShaderModule ShaderFactory::createShaderModule(const std::vector<char> &code)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(Device::VulkanDevice::getLogicalDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    std::vector<char> ShaderFactory::readFile(std::string fileName)
    {
        std::ifstream file{fileName, std::ios::binary};
        // 使用istream_iterator作为迭代器，执行结束后就把文件关闭了，不能再读取数据了
        // 使用istreambuf_iterator直接对缓冲区进行操作，效率会更高，并且执行结束不会影响文件的状态
        std::vector<char> code{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>{}};
        file.close();
        return code;
    }

    std::vector<VkFramebuffer> RenderPassFactory::swapChainFramebuffers;
    VkRenderPass RenderPassFactory::renderPass = VK_NULL_HANDLE;

    void RenderPassFactory::createRenderPass()
    {
        // 描述附件的使用和操作
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = Presentation::SwapChain::getSwapChainImageFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        // 多个子通道能够用于后处理等效果
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // 指定附件在内存中的布局
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef; // 数组的索引对应着色器中的layout(location = i) out vec3 fragColor;的i

        // subpass依赖图像附件的布局转换，这样定义的subpass依赖的作用是为了保证在开始渲染之前，
        // swapchain图像已经可用，并且图像布局已经转换为VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL。
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // 表示这里的布局转换是一种隐式转换，一般是从swapchain中获取时执行的转换
        dependency.dstSubpass = 0;  // 目标subpass是renderpass中的第一个pass
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // 同步subpas颜色附件的渲染，确保正确性
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(Device::VulkanDevice::getLogicalDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void RenderPassFactory::createFramebuffers()
    {
        using Presentation::SwapChain;
        swapChainFramebuffers.resize(SwapChain::getSwapChainImageViews().size());
        // 为每个交换链图像创建缓冲区，缓冲区可用于renderpass绘制图像
        for (size_t i = 0; i < SwapChain::getSwapChainImageViews().size(); i++)
        {
            VkImageView attachments[] = {
                SwapChain::getSwapChainImageViews()[i]};

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;    // 使用该缓冲区的renderPass
            framebufferInfo.attachmentCount = 1;        // 附件的数量
            framebufferInfo.pAttachments = attachments; // 缓冲区绑定的附件资源
            framebufferInfo.width = SwapChain::getSwapChainExtent().width;
            framebufferInfo.height = SwapChain::getSwapChainExtent().height;
            framebufferInfo.layers = 1; // 缓冲区的数量

            if (vkCreateFramebuffer(Device::VulkanDevice::getLogicalDevice(),
                                    &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    std::vector<VkFramebuffer> RenderPassFactory::getSwapChainFramebuffers()
    {
        return swapChainFramebuffers;
    }
    VkRenderPass RenderPassFactory::GetRenderPass()
    {
        if (renderPass == nullptr)
            createRenderPass();
        return renderPass;
    }

    void RenderPassFactory::cleanup()
    {
        vkDestroyRenderPass(Device::VulkanDevice::getLogicalDevice(), renderPass, nullptr);
    }

    VkPipelineLayout Pipeline::pipelineLayout;
    VkPipeline Pipeline::graphicsPipeline = VK_NULL_HANDLE;

    void Pipeline::createGraphicsPipeline()
    {
        std::unique_ptr<VkPipelineShaderStageCreateInfo[]> shaderStages = ShaderFactory::getDefaultShaderInfo();
        

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        auto bindingDescription = Mesh::SimpleMesh::Vertex::getBindingDescription();
        auto attributeDescriptions = Mesh::SimpleMesh::Vertex::getAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        // 为视口设置动态设置，然后在单个命令队列中设置不同的视口和裁剪矩阵
        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        // 如果没有设置动态状态，则需要在视口状态中传入视口和裁剪信息
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        // 光栅化状态设置
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;        // 设置近远平面之间的裁剪状态
        rasterizer.rasterizerDiscardEnable = VK_FALSE; // 设置是否开启光栅化
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // 图像光栅化的模式，包括：点，线，面
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;    // 设置表面提出的类型，正面剔除，背面提出等
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; // 表面的顶点位置方向（顶点朝前方向的顺时针）
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        // vkpipelinelayout用于指定uniform变量的描述，uniform可以用于所有着色器，并且可以在渲染期间动态改变
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        
        if (vkCreatePipelineLayout(Device::VulkanDevice::getLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages.get();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = RenderPassFactory::GetRenderPass();
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        VkResult error_code = vkCreateGraphicsPipelines(Device::VulkanDevice::getLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
        if (error_code != VK_SUCCESS)
        {
            std::cout<<error_code<<std::endl;
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        ShaderFactory::destroyShader();
    }

    VkPipeline Pipeline::getGraphicPipeline()
    {
        if(graphicsPipeline==VK_NULL_HANDLE)
            createGraphicsPipeline();
        return graphicsPipeline;
    }

    void Pipeline::cleanup()
    {
        vkDestroyPipeline(Device::VulkanDevice::getLogicalDevice(), graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(Device::VulkanDevice::getLogicalDevice(), pipelineLayout, nullptr);
    }
}