#define GLFW_INCLUDE_VULKAN
#define VulkanHeader
#include <GLFW/glfw3.h>


namespace Init{
    class GlfwWindow{
        GlfwWindow();
        GlfwWindow(const GlfwWindow&)=delete;
        GlfwWindow(const GlfwWindow&&)=delete;
        GlfwWindow& operator=(const GlfwWindow&)=delete;
    public:
        static void initWindow(int &error_code);
        static void loop();
        static void cleanup();
        static GLFWwindow* getGlfwWindow(){return glfw_window_handler;}
        const char ** GetExtensionInfo(uint32_t &glfwExtensionCount);
    
    private:
        static GLFWwindow* glfw_window_handler;
    };
}