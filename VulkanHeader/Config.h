#ifndef VulkanHeader
#define VulkanHeader
#include <vulkan/vulkan.h>
#endif
#include <vector>

namespace Config{
    enum class AreaWidthHeigh{
        Width=800,
        Height=600
        };
    
    extern bool enableValidationLayers;

    extern const std::vector<const char *> validationLayers;

    extern const std::vector<const char*> deviceExtensions;
    
    extern const int MAX_FRAMES_IN_FLIGHT;

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    bool checkValidationLayerSupport();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void setupDebugMessenger();
    void cleanup();

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
}