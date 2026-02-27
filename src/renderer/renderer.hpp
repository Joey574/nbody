#pragma once
#include <chrono>
#include <string>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../data/data.hpp"

struct renderer {
    public:

    std::chrono::nanoseconds render(const data& data);
    void cleanup();

    void init(size_t n);
    bool should_close() { return glfwWindowShouldClose(window); }
    void poll_events() { glfwPollEvents(); }

    private:
    GLFWwindow* window = nullptr;

    uint32_t graphicsIndex;
    uint32_t presentIndex;

    vk::raii::Context context;
    vk::raii::Instance instance = nullptr;
    vk::raii::SurfaceKHR surface = nullptr;
    vk::raii::Device device = nullptr;
    vk::raii::PhysicalDevice physicalDevice = nullptr;
    vk::raii::Queue graphicsQueue = nullptr;
    vk::raii::Queue presentQueue = nullptr;
    vk::SurfaceFormatKHR swapChainSurfaceFormat;
    vk::Extent2D swapChainExtent;
    vk::raii::SwapchainKHR swapChain = nullptr;
    std::vector<vk::Image> swapChainImages;
    vk::Format swapChainImageFormat = vk::Format::eUndefined;
    std::vector<const char*> deviceExtensions = { vk::KHRSwapchainExtensionName };
    std::vector<vk::raii::ImageView> swapChainImageViews;
    vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
    vk::raii::PipelineLayout pipelineLayout = nullptr;
    vk::raii::Pipeline graphicsPipeline = nullptr;
    vk::raii::CommandPool commandPool = nullptr;
    vk::raii::CommandBuffer commandBuffer = nullptr;

    void init_window();
    void vulkan_instance();
    void vulkan_surface();
    void vulkan_physicaldevice();
    void vulkan_device();
    void vulkan_swapchain();
    void vulkan_image_views();
    void vulkan_graphics_pipeline();
    void vulkan_command_pool();
    void vulkan_command_buffer();
    void vulkan_record_command_buffer(uint32_t imageIndex);
    void transition_image_layout(
        uint32_t imageIndex,
        vk::ImageLayout oldLayout,
        vk::ImageLayout newLayout,
        vk::AccessFlags2 srcAccessMask,
        vk::AccessFlags2 dstAccessMask,
        vk::PipelineStageFlags2 srcStageMask,
        vk::PipelineStageFlags2 dstStageMask    
    );

    std::vector<char> readFile(const std::string& path);
    [[nodiscard]] vk::raii::ShaderModule createShaderModule(const std::vector<char>& code);

    uint32_t findQueueFamilies(vk::raii::PhysicalDevice physicalDevice);
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats);
    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& presentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
    

    static const size_t width_ = 800;
    static const size_t height_ = 800;
};
