cmake_minimum_required(VERSION 3.0.0)
project(Vulkan VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O2")

add_executable(Vulkan VulkanMain.cpp)

target_link_libraries(Vulkan glfw vulkan dl pthread X11 Xxf86vm Xrandr Xi)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
