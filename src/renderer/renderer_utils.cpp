/*
Author: Joey Soroka
Purpose: Implement various helper functions used in the renderer struct
Comments:
*/

#include "renderer.hpp"
#include <fstream>
#include <vector>

vk::raii::ShaderModule renderer::createShaderModule(const std::vector<char>& code) {
    vk::ShaderModuleCreateInfo createInfo {
        .codeSize = code.size() * sizeof(char),
        .pCode = reinterpret_cast<const uint32_t*>(code.data())
    };

    vk::raii::ShaderModule shaderModule{ldevice.getDevice(), createInfo};
    return shaderModule;
}

uint32_t renderer::findQueueFamilies(vk::raii::PhysicalDevice physicalDevice) {
    // find the index of the first queue family that supports graphics
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

    // get the first index into queueFamilyProperties which supports graphics
    auto graphicsQueueFamilyProperty =
      std::find_if( queueFamilyProperties.begin(),
                    queueFamilyProperties.end(),
                    []( vk::QueueFamilyProperties const & qfp ) { return qfp.queueFlags & vk::QueueFlagBits::eGraphics; } );

    return static_cast<uint32_t>( std::distance( queueFamilyProperties.begin(), graphicsQueueFamilyProperty ) );
} 

std::vector<char> renderer::readFile(const std::string& path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        std::__throw_runtime_error(("failed to open file: " + path).c_str());
    }

    std::vector<char> buffer(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    file.close();

    return buffer;
}

std::vector<const char*> renderer::getRequiredInstanceExtensions() {
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    return extensions;
}

void renderer::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto r    = reinterpret_cast<renderer*>(glfwGetWindowUserPointer(window));
    r->framebufferResized = true;
}

uint32_t renderer::find_memory_type(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    auto memProperties = pdevice.get().getMemoryProperties();
    
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find a suitable memory type");
}