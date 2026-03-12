#pragma once
#include <chrono>
#include <cstdint>
#include <string>

#include "../definitions/graphics.hpp" // IWYU pragma: keep

#include "../graphics/logicaldevice/logicaldevice.hpp"
#include "../graphics/physicaldevice/physicaldevice.hpp"
#include "../graphics/swapchain/swapchain.hpp"

#include "../data/data.hpp"

struct vertex {
    glm::vec2 pos;
    glm::vec3 color;

    static vk::VertexInputBindingDescription getBindingDescription() {
        return {0, sizeof(vertex), vk::VertexInputRate::eVertex};
    }

    static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions() {
        return {
            vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32Sfloat, offsetof(vertex, pos)),
            vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(vertex, color))
        };
    }
};


struct renderer {
    public:

    std::chrono::nanoseconds render(const data& data);
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

    logicaldevice  ldevice;
    physicaldevice pdevice;
    swapchain      swapchain;

    vk::raii::PipelineLayout pipelineLayout   = nullptr;
    vk::raii::Pipeline       pipeline         = nullptr;
    vk::raii::CommandPool    commandPool      = nullptr;

    std::vector<vk::raii::CommandBuffer> commandBuffers;
    std::vector<vk::raii::Semaphore>     presentCompleteSemaphores;
    std::vector<vk::raii::Semaphore>     renderFinishedSemaphores;
    std::vector<vk::raii::Fence>         inFlightFences;
    uint32_t                             frameIndex;
    bool                                 framebufferResized = false;

    vk::raii::Buffer vertexBuffer = nullptr;
    vk::raii::DeviceMemory vertexBufferMemory = nullptr;

    std::vector<const char*> deviceExtensions = {
        vk::KHRSwapchainExtensionName
    };

    // TODO : used just for the vulkan tutorial stuff, remove later
    std::vector<vertex> vertices = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

    vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;

    void init_window();
    void vulkan_instance();
    void vulkan_surface();
    void vulkan_graphics_pipeline(const std::string& shaderPath);
    void vulkan_command_pool();
    void vulkan_command_buffer();
    void vulkan_sync_objects();
    void vulkan_vertex_buffer(const data& data);

    void vulkan_update_vertex_buffer(const data& data);
    
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
    uint32_t find_memory_type(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

    static std::vector<char> readFile(const std::string& path);
    static uint32_t findQueueFamilies(vk::raii::PhysicalDevice physicalDevice);
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    
    static const size_t width_ = 800;
    static const size_t height_ = 800;
};
