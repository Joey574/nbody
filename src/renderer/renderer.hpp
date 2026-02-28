#pragma once
#include <chrono>
#include <cstdint>
#include <string>
#include <vulkan/vulkan_core.h>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../data/data.hpp"

struct renderer {
    public:

    std::chrono::nanoseconds render(const data& data);
    void cleanup();

    void init(size_t n, const std::string& exePath);
    bool should_close() { return glfwWindowShouldClose(window); }
    void poll_events() { glfwPollEvents(); }

    private:
    static constexpr int MAX_FRAMES_IN_FLIGHT       = 2;
    GLFWwindow*                      window         = nullptr;
    vk::raii::Context                context;
    vk::raii::Instance               instance       = nullptr;
    vk::raii::SurfaceKHR             surface        = nullptr;
    vk::raii::PhysicalDevice         physicalDevice = nullptr;
    vk::raii::Device                 device         = nullptr;
    uint32_t                         queueIndex     = ~0;
    vk::raii::Queue                  queue          = nullptr;
    vk::raii::SwapchainKHR           swapChain      = nullptr;
    std::vector<vk::Image>           swapChainImages;
    vk::SurfaceFormatKHR             swapChainSurfaceFormat;
    vk::Extent2D                     swapChainExtent;
    std::vector<vk::raii::ImageView> swapChainImageViews;
    
    vk::raii::PipelineLayout pipelineLayout   = nullptr;
    vk::raii::Pipeline       pipeline         = nullptr;
    vk::raii::CommandPool    commandPool      = nullptr;

    std::vector<vk::raii::CommandBuffer> commandBuffers;
    std::vector<vk::raii::Semaphore>     presentCompleteSemaphores;
    std::vector<vk::raii::Semaphore>     renderFinishedSemaphores;
    std::vector<vk::raii::Fence>         inFlightFences;
    uint32_t                             frameIndex;

    std::vector<const char*> deviceExtensions = {
        vk::KHRSwapchainExtensionName
    };

    vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;

    void init_window();
    void vulkan_instance();
    void vulkan_surface();
    void vulkan_physicaldevice();
    void vulkan_device();
    void vulkan_swapchain();
    void vulkan_image_views();
    void vulkan_graphics_pipeline(const std::string& shaderPath);
    void vulkan_command_pool();
    void vulkan_command_buffer();
    void vulkan_sync_objects();
    
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
    std::vector<const char*> getRequiredInstanceExtensions();
    vk::raii::ShaderModule createShaderModule(const std::vector<char>& code);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

    static std::vector<char> readFile(const std::string& path);
    static uint32_t chooseSwapMinImageCount(const vk::SurfaceCapabilitiesKHR& capabilities);
    static uint32_t findQueueFamilies(vk::raii::PhysicalDevice physicalDevice);
    static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats);
    static vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& presentModes);
    
    static const size_t width_ = 800;
    static const size_t height_ = 800;
};
