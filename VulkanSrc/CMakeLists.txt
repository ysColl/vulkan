cmake_minimum_required(VERSION 3.0.0)

project(VulkanSrc VERSION 0.1.0 LANGUAGES C CXX)

aux_source_directory(. SrcFiles)
message("SrcFiles: ${SrcFiles}")

add_library(VulkanSrc STATIC ${SrcFiles})
