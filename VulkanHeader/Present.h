#ifndef VulkanHeader
#define VulkanHeader
#include <vulkan/vulkan.h>
#endif

#include <vector>

namespace Presentation{
    class SwapChain{
        static void createSwapChain();
        static void createImageViews();
    public:
        static void DoInit();
        static void cleanup();
        static void recreateSwapChain();
        static VkSwapchainKHR getSwapChain();
        static VkFormat getSwapChainImageFormat();
        static VkExtent2D getSwapChainExtent();
        static std::vector<VkImageView> getSwapChainImageViews();

    private:
        static VkFormat swapChainImageFormat;
        static std::vector<VkImageView> swapChainImageViews;
        static VkExtent2D swapChainExtent;
        static VkSwapchainKHR swapChain;
        static std::vector<VkImage> swapChainImages;
    };
}