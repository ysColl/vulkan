#include "MeshData.h"
#include "Device.h"
#include "Draw.h"

#include <stdexcept>
#include <cstring>


namespace Mesh{
    VkBuffer SimpleMesh::vertexBuffer;
    VkDeviceMemory SimpleMesh::vertexBufferMemory;
    VkBuffer SimpleMesh::indexBuffer;
    VkDeviceMemory SimpleMesh::indexBufferMemory;

    void DoInit(){
        SimpleMesh::createVertexBuffer();
        SimpleMesh::createIndexBuffer();
    }

    const std::vector<SimpleMesh::Vertex> SimpleMesh::vertices = {
        {{-0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}},
        {{0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}},
        {{0.5f, 0.0f}, {0.5f, 0.5f, 0.5f}},
        {{-0.5f, 0.0f}, {0.5f, 0.5f, 0.5f}},

        {{-0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
        {{0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
        {{0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    };

    const std::vector<uint16_t> SimpleMesh::indices = {
        0, 1, 2,
        2, 3, 0,
        4,5,6,
        6,7,4
    };

    void SimpleMesh::cleanup(){
        vkDestroyBuffer(Device::VulkanDevice::getLogicalDevice(), indexBuffer, nullptr);
        vkFreeMemory(Device::VulkanDevice::getLogicalDevice(), indexBufferMemory, nullptr);

        vkDestroyBuffer(Device::VulkanDevice::getLogicalDevice(), vertexBuffer, nullptr);
        vkFreeMemory(Device::VulkanDevice::getLogicalDevice(), vertexBufferMemory, nullptr);
    }

    void SimpleMesh::createVertexBuffer() {
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory; // 暂存缓冲区内存可能无法被cpu访问，需要通过cpu映射来执行访问
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        // 将暂存缓冲区的内存映射到cpu可访问的内存指针
        void* data;
        vkMapMemory(Device::VulkanDevice::getLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), (size_t) bufferSize);  // 复制顶点数据到暂存缓冲区
        vkUnmapMemory(Device::VulkanDevice::getLogicalDevice(), stagingBufferMemory);
        // 创建GPU内存，用于顶点数据处理，仅GPU访问的内存，读写效率更高
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

        copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

        vkDestroyBuffer(Device::VulkanDevice::getLogicalDevice(), stagingBuffer, nullptr);
        vkFreeMemory(Device::VulkanDevice::getLogicalDevice(), stagingBufferMemory, nullptr);
    }

    void SimpleMesh::createIndexBuffer() {
        VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        //它的内存属性是主机可见和主机一致，即可以被CPU访问和修改，并且不需要显式刷新或使缓存失效
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(Device::VulkanDevice::getLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, indices.data(), (size_t) bufferSize);
        vkUnmapMemory(Device::VulkanDevice::getLogicalDevice(), stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

        copyBuffer(stagingBuffer, indexBuffer, bufferSize);

        vkDestroyBuffer(Device::VulkanDevice::getLogicalDevice(), stagingBuffer, nullptr);
        vkFreeMemory(Device::VulkanDevice::getLogicalDevice(), stagingBufferMemory, nullptr);
    }

    void SimpleMesh::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        // 创建命令缓冲区，并传输内存复制命令，最后提交命令
        auto device = Device::VulkanDevice::getLogicalDevice();
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = DrawSpace::CommondFactory::getCommandPool();
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; //一次性的提交

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(Device::VulkanDevice::getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(Device::VulkanDevice::getGraphicsQueue());

        vkFreeCommandBuffers(device, DrawSpace::CommondFactory::getCommandPool(), 1, &commandBuffer);
    }

    void SimpleMesh::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;  // 数据的用途，这里可能是顶点数据或索引数据
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  // 缓冲区是否能够被多个设备使用

        auto device = Device::VulkanDevice::getLogicalDevice();

        // 创建缓冲区对象
        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        // 获取缓冲区的数据格式需求
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        //它根据内存需求结构体中的兼容的内存类型位图和期望的内存属性（例如设备本地、主机可见等）来选择一个合适的内存类型。
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }

    uint32_t SimpleMesh::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(Device::VulkanDevice::getPhysicalDevice(), &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }
}