cmake_minimum_required(VERSION 3.0.0)
project(Vulkan VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

include_directories(./VulkanHeader)
add_subdirectory(./VulkanSrc)

add_executable(Vulkan VulkanMain.cpp)
# 依赖项的顺序要按照依赖顺序，（库，被依赖库，被依赖库2，库，被依赖库）
target_link_libraries(Vulkan VulkanSrc glfw vulkan dl pthread X11 Xxf86vm Xrandr Xi) 

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
