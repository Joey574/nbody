#pragma once
#include <chrono>
#include <cstdint>
#include <string>

#include "../definitions/graphics.hpp" // IWYU pragma: keep

#include "../graphics/LogicalDevice/LogicalDevice.hpp"
#include "../graphics/PhysicalDevice/PhysicalDevice.hpp"
#include "../graphics/Swapchain/Swapchain.hpp"
#include "../graphics/CommandBuffer/CommandBuffer.hpp"
#include "../graphics/FrameData/FrameData.hpp"
#include "../graphics/UBOBuffer/UBOBuffer.hpp"

#include "../camera/camera.hpp"
#include "../data/data.hpp"

struct renderer {
    public:
    std::chrono::nanoseconds render(const data& data, float dt);
    void cleanup();

    void init(const data& data, const std::string& exePath);
    bool should_close() { return glfwWindowShouldClose(window); }
    void poll_events() { glfwPollEvents(); }

    private:
    static constexpr int MAX_FRAMES_IN_FLIGHT       = 2;
    GLFWwindow*                      window         = nullptr;
    vk::raii::Context                context;
    vk::raii::Instance               instance       = nullptr;
    vk::raii::SurfaceKHR             surface        = nullptr;

    camera         cam;
    LogicalDevice  ldevice;
    PhysicalDevice pdevice;
    Swapchain      swapchain;

    CommandBuffer command;
    FrameData frames[MAX_FRAMES_IN_FLIGHT];
    UBOBuffer uboBuffers[MAX_FRAMES_IN_FLIGHT];

    vk::raii::PipelineLayout pipelineLayout   = nullptr;
    vk::raii::Pipeline       pipeline         = nullptr;
    vk::raii::CommandPool    commandPool      = nullptr;

    std::vector<vk::raii::CommandBuffer> commandBuffers;
    std::vector<vk::raii::Semaphore>     presentCompleteSemaphores;
    std::vector<vk::raii::Semaphore>     renderFinishedSemaphores;
    std::vector<vk::raii::Fence>         inFlightFences;
    uint32_t                             frameIndex;
    bool                                 framebufferResized = false;

    std::vector<const char*> deviceExtensions = { vk::KHRSwapchainExtensionName };
    vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;

    
    vk::raii::DescriptorPool descriptorPool = nullptr;
    vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
    vk::raii::DescriptorSets descriptorSets = nullptr;


    void init_window();
    void vulkan_instance();
    void vulkan_surface();
    void vulkan_graphics_pipeline();
    void vulkan_command_pool();
    void vulkan_command_buffer();
    void vulkan_sync_objects();
    void vulkan_init_descriptors();
    void vulkan_write_descriptors(size_t i);

    void vulkan_record_command_buffer(uint32_t imageIndex, size_t n);
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
    vk::raii::ShaderModule createShaderModule(const char* code, const size_t size);
    
    static uint32_t chooseSwapMinImageCount(const vk::SurfaceCapabilitiesKHR& capabilities);
    static uint32_t findQueueFamilies(vk::raii::PhysicalDevice physicalDevice);
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    static constexpr const unsigned char shader_bytes[] = { 
        #embed "../shaders/circles.spv"
    };
    static constexpr const size_t shader_size = sizeof(renderer::shader_bytes);
    
    static const size_t width_ = 800;
    static const size_t height_ = 800;
};
