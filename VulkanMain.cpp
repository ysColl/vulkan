#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>


#include "window.h"
#include "Instance.h"
#include "Device.h"
#include "Present.h"
#include "Draw.h"
#include "PipelineData.h"
#include "Config.h"


int main(){
    
    try {
        int error_code = 0;
        Init::GlfwWindow::initWindow(error_code);
        VkResult vk_error_code;
        Init::Instance::CreateInstance(vk_error_code);
        Config::setupDebugMessenger();
        Device::DoInit();
        Presentation::SwapChain::DoInit();
        PipelineData::DoInit();
        DrawSpace::CommondFactory::DoInit();

        Init::GlfwWindow::loop();
        
        Presentation::SwapChain::cleanup();
        PipelineData::cleanup();
        DrawSpace::CommondFactory::cleanup();
        Device::VulkanDevice::cleanup();
        Init::Instance::cleanup();
        Init::GlfwWindow::cleanup();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
