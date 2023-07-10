#include "Draw.h"
#include "Device.h"
#include "PipelineData.h"
#include "Present.h"
#include "Config.h"
#include "MeshData.h"

#include <stdexcept>


namespace DrawSpace{
    void CommondFactory::DoInit(){
        createCommandPool();
        Mesh::DoInit();
        createCommandBuffers();
        createSyncObjects();
    }


    VkCommandPool CommondFactory::commandPool;
    std::vector<VkCommandBuffer> CommondFactory::commandBuffers;
    std::vector<VkSemaphore> CommondFactory::imageAvailableSemaphores;
    std::vector<VkSemaphore> CommondFactory::renderFinishedSemaphores;
    std::vector<VkFence> CommondFactory::inFlightFences;

    VkCommandPool CommondFactory::getCommandPool(){
        return commandPool;
    }

    void CommondFactory::createCommandPool() {
        Device::QueueFamilyIndices queueFamilyIndices =
         Device::VulkanDevice::findQueueFamilies(Device::VulkanDevice::getPhysicalDevice());

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;  // 允许命令缓冲区单独重置
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();  // 每个命令缓冲区池只能

        if (vkCreateCommandPool(Device::VulkanDevice::getLogicalDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    void CommondFactory::createCommandBuffers() {
        commandBuffers.resize(Config::MAX_FRAMES_IN_FLIGHT);

        // 指定命令缓冲区分配器的设置
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;  // 分配为主缓冲区还是次级缓冲区
        allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

        if (vkAllocateCommandBuffers(Device::VulkanDevice::getLogicalDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void CommondFactory::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        // 用于设置命令缓冲区的使用方法，当前命令缓冲区状态，如何继承主命令缓冲区的状态
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        // 调用begin将重置命令缓冲区，并且可以向缓冲区中设置命令
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
        VkExtent2D swapChainExtent = Presentation::SwapChain::getSwapChainExtent();
        // 开始一个render pass实例，指定使用哪个render pass对象和framebuffer对象。
        // 遍历render pass中的每个subpass，记录该subpass的渲染命令。
        // 结束render pass实例，完成渲染过程。
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = PipelineData::RenderPassFactory::GetRenderPass();
        std::vector<VkFramebuffer> swapChainFramebuffers = PipelineData::RenderPassFactory::getSwapChainFramebuffers();
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];  // 绑定用于绘制的图像缓冲区
        // 当前资源中，着色器所需要绘制的区域
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChainExtent;
        // 重置当前缓冲区所使用的颜色信息
        VkClearValue clearColor = {{{0.25f, 0.25f, 0.25f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        // 传输vkCmd*命令，绘制图像
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); // 记录renderPass中第一个subpass的命令，指定了颜色附件
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineData::Pipeline::getGraphicPipeline());

        // 视口定义了窗口中图像显示的区域
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) swapChainExtent.width;
        viewport.height = (float) swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        // 裁剪定义了显示区域的能够显示的部分
        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        // vkCmdDraw函数接受以下几个参数：

        // commandBuffer，表示要记录命令的command buffer对象。
        // vertexCount，表示要绘制的顶点数量。
        // instanceCount，表示要绘制的实例数量。实例是指使用相同的顶点数据和管线状态来绘制多个图形的一种技术，可以提高性能和效果。
        // firstVertex，表示从顶点缓冲区中的哪个位置开始读取顶点数据。
        // firstInstance，表示从实例缓冲区中的哪个位置开始读取实例数据。实例缓冲区是一种存储每个实例的特定数据的缓冲区，例如变换矩阵或者颜色
        // vkCmdDraw(commandBuffer, 3, 1, 0, 0);

        VkBuffer vertexBuffers[] = {Mesh::SimpleMesh::getVertexBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, Mesh::SimpleMesh::getIndexBuffer(), 0, VK_INDEX_TYPE_UINT16);

        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(Mesh::SimpleMesh::getIndices().size()), 1, 0, 0, 0);

        vkCmdEndRenderPass(commandBuffer);
        // 结束命令传输，下一步可以执行提交命令
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void CommondFactory::createSyncObjects() {
        imageAvailableSemaphores.resize(Config::MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(Config::MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(Config::MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < Config::MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(Device::VulkanDevice::getLogicalDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(Device::VulkanDevice::getLogicalDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(Device::VulkanDevice::getLogicalDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    void CommondFactory::drawFrame() {
        static u_int32_t currentFrame = 1;
        // 等待上一帧的绘制命令是否完成，即当前命令缓冲区是否可用。可用的话就重置fence
        vkWaitForFences(Device::VulkanDevice::getLogicalDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        vkResetFences(Device::VulkanDevice::getLogicalDevice(), 1, &inFlightFences[currentFrame]);

        // 获取交换链中下一个可用的图像，并在相应图像缓冲区中执行绘制操作
        uint32_t imageIndex;
        vkAcquireNextImageKHR(Device::VulkanDevice::getLogicalDevice(), Presentation::SwapChain::getSwapChain(),
                             UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        // 重置命令缓冲区，并传输命令
        vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;  // 设置在哪个阶段等待信号量，当前值表示在颜色附件输出阶段等待从交换链中获取图像缓冲区

        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;  // 设置执行完成后点亮的信号量
        // 提交队列中的命令缓冲区，并设置fence用于命令执行结束后执行绘制下一帧
        if (vkQueueSubmit(Device::VulkanDevice::getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores; // 等待图像命令执行完成

        VkSwapchainKHR swapChains[] = {Presentation::SwapChain::getSwapChain()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        vkQueuePresentKHR(Device::VulkanDevice::getPresentQueue(), &presentInfo);

        currentFrame = (currentFrame + 1) % Config::MAX_FRAMES_IN_FLIGHT;
    }

    void CommondFactory::cleanup(){
        Mesh::SimpleMesh::cleanup();
        for (size_t i = 0; i < Config::MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(Device::VulkanDevice::getLogicalDevice(), renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(Device::VulkanDevice::getLogicalDevice(), imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(Device::VulkanDevice::getLogicalDevice(), inFlightFences[i], nullptr);
        }
        vkDestroyCommandPool(Device::VulkanDevice::getLogicalDevice(), commandPool, nullptr);
    }

}