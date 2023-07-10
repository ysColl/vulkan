#include "window.h"
#include "Config.h"
#include "Draw.h"


namespace Init
{
    GLFWwindow *GlfwWindow::glfw_window_handler = nullptr;

    void GlfwWindow::initWindow(int &error_code)
    {
        glfwInit();
        // 禁用Opengl上下文和重置窗口大小
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        using Config::AreaWidthHeigh;
        glfw_window_handler = glfwCreateWindow((int)AreaWidthHeigh::Width, (int)AreaWidthHeigh::Height, "Vulkan", nullptr, nullptr);
    }

    const char** GlfwWindow::GetExtensionInfo(uint32_t &glfwExtensionCount)
    {
        return glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    }

    void GlfwWindow::loop()
    {
        while (!glfwWindowShouldClose(glfw_window_handler)) {
            glfwPollEvents();
            DrawSpace::CommondFactory::drawFrame();
        }
    }
    void GlfwWindow::cleanup()
    {
        glfwDestroyWindow(glfw_window_handler);
        glfwTerminate();
    }
}