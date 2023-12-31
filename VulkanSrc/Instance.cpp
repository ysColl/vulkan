#include "Instance.h"
#include "Config.h"
#include <GLFW/glfw3.h>
#include <stdexcept>


namespace Init{
    VkInstance Instance::vulkanInstance;

    std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (Config::enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    void Instance::CreateInstance(VkResult &error_code){
        if (Config::enableValidationLayers && !Config::checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (Config::enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(Config::validationLayers.size());
            createInfo.ppEnabledLayerNames = Config::validationLayers.data();

            Config::populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &vulkanInstance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    };

    VkInstance& Instance::GetInstance(){
        if(vulkanInstance==VK_NULL_HANDLE)
        {
            VkResult error_code;
            CreateInstance(error_code);
        }
        return vulkanInstance;
    }
}