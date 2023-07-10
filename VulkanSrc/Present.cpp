#include "Present.h"
#include "Device.h"
#include "Config.h"
#include "window.h"
#include "PipelineData.h"

#include <limits>
#include <algorithm>
#include <GLFW/glfw3.h>
#include <system_error>
#include <set>


namespace Presentation{
    VkSwapchainKHR SwapChain::swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> SwapChain::swapChainImages{};
    VkFormat SwapChain::swapChainImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D SwapChain::swapChainExtent;
    std::vector<VkImageView> SwapChain::swapChainImageViews{};
    
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(Init::GlfwWindow::getGlfwWindow(), &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }
    
    void SwapChain::createSwapChain() {
        // 获取surface支持的swapchain的功能,格式和显示模式
        Config::SwapChainSupportDetails swapChainSupport = Config::querySwapChainSupport(Device::VulkanDevice::getPhysicalDevice(), Device::VulkanDevice::getSurface());
        // 获取合适的swapchain功能,格式和显示模式
        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }
        // 开始创建交换链
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = Device::VulkanDevice::getSurface();

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        Device::QueueFamilyIndices indices =
         Device::VulkanDevice::findQueueFamilies(Device::VulkanDevice::getPhysicalDevice());
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = VK_NULL_HANDLE;
        if (vkCreateSwapchainKHR(Device::VulkanDevice::getLogicalDevice(), &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        // 获取交换链用于呈现图像的图像数组
        vkGetSwapchainImagesKHR(Device::VulkanDevice::getLogicalDevice(), swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(Device::VulkanDevice::getLogicalDevice(), swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    void SwapChain::createImageViews() {
        // 创建imageview,imgeview包含处理image的数据和方法
        swapChainImageViews.resize(swapChainImages.size());

        for (size_t i = 0; i < swapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(Device::VulkanDevice::getLogicalDevice(), &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image views!");
            }
        }
    }

    void SwapChain::recreateSwapChain() {
        int width = 0, height = 0;
        glfwGetFramebufferSize(Init::GlfwWindow::getGlfwWindow(), &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(Init::GlfwWindow::getGlfwWindow(), &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(Device::VulkanDevice::getLogicalDevice());

        cleanup();

        createImageViews();
        createImageViews();
        PipelineData::RenderPassFactory::createFramebuffers();
    }

    void SwapChain::DoInit(){
        createSwapChain();
        createImageViews();
    }

    void SwapChain::cleanup(){
        for (auto framebuffer : PipelineData::RenderPassFactory::getSwapChainFramebuffers()) {
            vkDestroyFramebuffer(Device::VulkanDevice::getLogicalDevice(), framebuffer, nullptr);
        }
        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(Device::VulkanDevice::getLogicalDevice(), imageView, nullptr);
        }
        vkDestroySwapchainKHR(Device::VulkanDevice::getLogicalDevice(), swapChain, nullptr);
    }
    VkFormat SwapChain::getSwapChainImageFormat(){
        return swapChainImageFormat;
    }

    VkExtent2D SwapChain::getSwapChainExtent()
    {
        return swapChainExtent;
    }
    std::vector<VkImageView> SwapChain::getSwapChainImageViews()
    {
        return swapChainImageViews;
    }
}

VkSwapchainKHR Presentation::SwapChain::getSwapChain()
{
    if(swapChain==VK_NULL_HANDLE)
        createSwapChain();
    return swapChain;
}
