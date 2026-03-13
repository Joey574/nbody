/*
Author: Joey Soroka
Purpose: Implement various helper functions used in the renderer struct
Comments:
*/

#include "renderer.hpp"
#include <vector>

vk::raii::ShaderModule renderer::createShaderModule(const char* code, const size_t size) {
    vk::ShaderModuleCreateInfo createInfo {
        .codeSize = size * sizeof(char),
        .pCode = reinterpret_cast<const uint32_t*>(code)
    };

    vk::raii::ShaderModule shaderModule{ldevice, createInfo};
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
