/*
Author: Joey Soroka
Purpose: Implement various helper functions used in the renderer struct
Comments:
*/

#include "renderer.hpp"
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

vk::SurfaceFormatKHR renderer::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats) {
    for (const auto& f : formats) {
        if (f.format == vk::Format::eB8G8R8A8Srgb && f.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return f;
        }
    }

    return formats[0];
}

vk::PresentModeKHR renderer::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& presentModes) {
    bool hasImmediate = false;
    bool hasMailbox = false;

    for (const auto& mode : presentModes) {
        if (mode == vk::PresentModeKHR::eImmediate) {
            hasImmediate = true;
        } else if (mode == vk::PresentModeKHR::eMailbox) {
            hasMailbox = true;
        }
    }

    return hasImmediate ? vk::PresentModeKHR::eImmediate : 
           hasMailbox   ? vk::PresentModeKHR::eMailbox   :
           vk::PresentModeKHR::eFifo;
}

vk::Extent2D renderer::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);

    return {
        std::clamp<uint32_t>(w, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp<uint32_t>(h, capabilities.minImageExtent.height, capabilities.maxImageExtent.height),
    };
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