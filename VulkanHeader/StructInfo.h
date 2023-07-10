#ifndef VulkanHeader
#define VulkanHeader
#include <vulkan/vulkan.h>
#endif
#include <vector>

namespace VulkanInfo{
    void getDefaultVkInfoByType(VkStructureType type, void*);
}