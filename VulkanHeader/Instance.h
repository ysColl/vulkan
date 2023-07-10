#ifndef VulkanHeader
#define VulkanHeader
#include <vulkan/vulkan.h>
#endif


namespace Init{
    class Instance{
        Instance();
        Instance(const Instance&)=delete;
        Instance(const Instance&&)=delete;
        Instance& operator=(const Instance&)=delete;
    public:
        static void CreateInstance(VkResult &error_code);
        static VkInstance& GetInstance();
        static void cleanup(){vkDestroyInstance(vulkanInstance, nullptr);}
    
    private:
        static VkInstance vulkanInstance;
    };
}