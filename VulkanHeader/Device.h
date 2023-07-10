#ifndef VulkanHeader
#define VulkanHeader
#include <vulkan/vulkan.h>
#endif
#include <optional>


namespace Device{
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    void DoInit();

    class VulkanDevice{
        VulkanDevice(){};
        VulkanDevice(const VulkanDevice&)=delete;
        VulkanDevice(const VulkanDevice&&)=delete;
        VulkanDevice& operator=(const VulkanDevice&)=delete;

        static bool isDeviceSuitable(VkPhysicalDevice device);
        static bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    public:
        static void CreateSurface();
        static void pickPhysicalDevice();
        static void createLogicalDevice();

        static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        static VkDevice& getLogicalDevice();
        static VkSurfaceKHR& getSurface();
        static VkPhysicalDevice& getPhysicalDevice();
        static VkQueue getGraphicsQueue();
        static VkQueue getPresentQueue();
        static void cleanup();

    private:
        static VkQueue graphicsQueue;
        static VkQueue presentQueue;

        static VkPhysicalDevice physicalDevice;  // 逻辑设备,主机上支持的vk设备版本
        static VkDevice device; // 逻辑设备,用来实例化一个物理设备实例
        static VkSurfaceKHR surface;
    };
}