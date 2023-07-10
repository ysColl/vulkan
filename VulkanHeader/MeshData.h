#ifndef VulkanHeader
#define VulkanHeader
#include <vulkan/vulkan.h>
#endif

#include <glm/glm.hpp>

#include <array>
#include <vector>

namespace Mesh{
    void DoInit();
    class SimpleMesh{

    public:
        struct Vertex {
            glm::vec2 pos;
            glm::vec3 color;

            static VkVertexInputBindingDescription getBindingDescription() {
                VkVertexInputBindingDescription bindingDescription{};
                bindingDescription.binding = 0;  // 顶点输入时，指定数据所在的顶点缓冲区
                bindingDescription.stride = sizeof(Vertex);
                bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // 顶点绑定的输入速率，这里表示每个顶点使用一个顶点数据

                return bindingDescription;
            }

            // 顶点数据中属性描述
            static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
                std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

                attributeDescriptions[0].binding = 0; // 与顶点绑定信息中的绑定信息相一致
                attributeDescriptions[0].location = 0;  // 在着色器中绑定的位置
                attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT; //数据格式，二维向量
                attributeDescriptions[0].offset = offsetof(Vertex, pos);

                attributeDescriptions[1].binding = 0;
                attributeDescriptions[1].location = 1;
                attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; //数据格式，三维向量
                attributeDescriptions[1].offset = offsetof(Vertex, color);

                return attributeDescriptions;
            }
        };
        static void createVertexBuffer();
        static void createIndexBuffer();
        static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        static void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

        static void cleanup();
        static VkBuffer& getVertexBuffer(){
            return vertexBuffer;
        }
        static VkBuffer& getIndexBuffer(){
            return indexBuffer;
        }
        static const std::vector<uint16_t>& getIndices(){
            return indices;
        }

    private:
        static const std::vector<Vertex> vertices;
        static const std::vector<uint16_t> indices;

        static VkBuffer vertexBuffer;
        static VkDeviceMemory vertexBufferMemory;
        static VkBuffer indexBuffer;
        static VkDeviceMemory indexBufferMemory;
    };
}