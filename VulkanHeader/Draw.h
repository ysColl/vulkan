#ifndef VulkanHeader
#define VulkanHeader
#include <vulkan/vulkan.h>
#endif

#include <vector>


namespace DrawSpace{
    class CommondFactory{
        static void createSyncObjects();
    public:
        static void createCommandPool();
        static void createCommandBuffers();
        static void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
        static void drawFrame();
        static void cleanup();
        static void DoInit();

        static VkCommandPool getCommandPool();
        
    private:
        static VkCommandPool commandPool;
        static std::vector<VkCommandBuffer> commandBuffers;
        static std::vector<VkSemaphore> imageAvailableSemaphores;
        static std::vector<VkSemaphore> renderFinishedSemaphores;
        static std::vector<VkFence> inFlightFences;
    };
}
