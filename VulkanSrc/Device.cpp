#include "Device.h"
#include "Instance.h"
#include "Config.h"
#include "window.h"

#include <stdexcept>
#include <vector>
#include <set>

namespace Device
{

    VkPhysicalDevice VulkanDevice::physicalDevice = VK_NULL_HANDLE; // 逻辑设备,主机上支持的vk设备版本
    VkDevice VulkanDevice::device = VK_NULL_HANDLE;
    VkSurfaceKHR VulkanDevice::surface = VK_NULL_HANDLE;
    VkQueue VulkanDevice::graphicsQueue = VK_NULL_HANDLE;
    VkQueue VulkanDevice::presentQueue = VK_NULL_HANDLE;

    void DoInit()
    {
        VulkanDevice::CreateSurface();
        VulkanDevice::pickPhysicalDevice();
        VulkanDevice::createLogicalDevice();
    }

    bool VulkanDevice::isDeviceSuitable(VkPhysicalDevice device)
    {
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        if (!deviceFeatures.fillModeNonSolid)
        {
            // 不支持非实心填充模式，只能使用VK_POLYGON_MODE_FILL
            return false;
        }

        QueueFamilyIndices indices = findQueueFamilies(device);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            Config::SwapChainSupportDetails swapChainSupport = Config::querySwapChainSupport(device, surface);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    bool VulkanDevice::checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(Config::deviceExtensions.begin(), Config::deviceExtensions.end());

        for (const auto &extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    QueueFamilyIndices VulkanDevice::findQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto &queueFamily : queueFamilies)
        {
            // 获取图形队列族
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }
            // 判断队列是否支持surface对象
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (presentSupport)
            {
                indices.presentFamily = i;
            }

            if (indices.isComplete())
            {
                break;
            }

            i++;
        }

        return indices;
    }

    void VulkanDevice::CreateSurface()
    {
        // surface是用于连接vulkan和window系统的中间层,可以用于vulkan呈现画面
        if (glfwCreateWindowSurface(Init::Instance::GetInstance(), Init::GlfwWindow::getGlfwWindow(), nullptr, &surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void VulkanDevice::pickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        VkInstance instance = Init::Instance::GetInstance();
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0)
        {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto &device : devices)
        {
            // 选择合适的物理设备
            if (isDeviceSuitable(device))
            {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE)
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    void VulkanDevice::createLogicalDevice()
    {
        // 创建队列信息,用于逻辑设备创建队列
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        // 创建两个队列信息，图形队列和展示队列信息
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            // 队列使用优先级，使用相同的优先级
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // 设置逻辑设备创建信息
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        // 添加设备扩展，如：swapchain扩展
        createInfo.enabledExtensionCount = static_cast<uint32_t>(Config::deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = Config::deviceExtensions.data();

        // createInfo.enabledExtensionCount = 0;  这里复制粘贴代码的时候少删了，导致查了很久，不知道是什么问题

        if (Config::enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(Config::validationLayers.size());
            createInfo.ppEnabledLayerNames = Config::validationLayers.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    }

    VkDevice &VulkanDevice::getLogicalDevice()
    {
        if (device == VK_NULL_HANDLE)
            DoInit();
        return device;
    }

    VkSurfaceKHR &VulkanDevice::getSurface()
    {
        if (surface == VK_NULL_HANDLE)
            CreateSurface();
        return surface;
    }

    VkPhysicalDevice &VulkanDevice::getPhysicalDevice()
    {
        if (physicalDevice == VK_NULL_HANDLE)
        {
            CreateSurface();
            pickPhysicalDevice();
        }
        return physicalDevice;
    }

    VkQueue VulkanDevice::getGraphicsQueue()
    {
        return graphicsQueue;
    }
    VkQueue VulkanDevice::getPresentQueue()
    {
        return presentQueue;
    }
    void VulkanDevice::cleanup()
    {
        vkDestroyDevice(device, nullptr);
        Config::cleanup();
        vkDestroySurfaceKHR(Init::Instance::GetInstance(), surface, nullptr);
    }
}