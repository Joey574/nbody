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

uint32_t renderer::find_memory_type(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    auto memProperties = pdevice.Device().getMemoryProperties();
    
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find a suitable memory type");
}

void renderer::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer& buffer, vk::raii::DeviceMemory& bufferMemory) {
    vk::BufferCreateInfo bufferInfo {
        .size = size,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive,
    };

    buffer = vk::raii::Buffer(ldevice, bufferInfo);
    
    auto memRequirements = buffer.getMemoryRequirements();
    vk::MemoryAllocateInfo allocInfo {
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = find_memory_type(memRequirements.memoryTypeBits, properties)
    };

    bufferMemory = vk::raii::DeviceMemory(ldevice, allocInfo);
    buffer.bindMemory(*bufferMemory, 0);
}

void renderer::copyBuffer(vk::raii::Buffer& src, vk::raii::Buffer& dst, vk::DeviceSize size) {
    vk::CommandBufferAllocateInfo allocInfo {
        .commandPool = commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };
    auto command = std::move(ldevice.Device().allocateCommandBuffers(allocInfo).front());
    command.begin(vk::CommandBufferBeginInfo {
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    });

    command.copyBuffer(src, dst, vk::BufferCopy(0, 0, size));
    command.end();

    ldevice.Queue().submit(vk::SubmitInfo {
        .commandBufferCount = 1,
        .pCommandBuffers = &*command
    }, nullptr);
    ldevice.Queue().waitIdle();
}