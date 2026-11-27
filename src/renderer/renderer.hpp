#pragma once

#include "definitions/graphics.hpp" // IWYU pragma: keep

#include "graphics/LogicalDevice/LogicalDevice.hpp"
#include "graphics/PhysicalDevice/PhysicalDevice.hpp"
#include "graphics/Swapchain/Swapchain.hpp"
#include "graphics/CommandBuffer/CommandBuffer.hpp"
#include "graphics/FrameData/FrameData.hpp"
#include "graphics/UBOBuffer/UBOBuffer.hpp"
#include "graphics/FrameCapture/FrameCapture.hpp"
#include "graphics/VideoEncoder/VideoEncoder.hpp"

#include "camera/camera.hpp"
#include "interactions/interactions.hpp"
#include "simulation/simulation.hpp"

#include <chrono>
#include <cstdint>
#include <string>

struct renderer {
    public:
        renderer() = default;
        renderer(const renderer&) = delete;
        renderer& operator=(const renderer&) = delete;
        ~renderer() { cleanup(); }

        void cleanup();
        void waitIdle();

        void releaseSimulationReferences();

        LogicalDevice& logicalDevice() noexcept { return ldevice; }
        PhysicalDevice& physicalDevice() noexcept { return pdevice; }

        std::chrono::nanoseconds render(simulation& sim, float dt, float ft);

        void initVulkan(const cliargs& cli);
        void initRendering(const simulation& sim);

        bool shouldClose() { return window == nullptr || glfwWindowShouldClose(window) || recordLimitReached(); }

        void enableRecording(std::string path, uint32_t fps, size_t maxFrames) {
            recordPath = std::move(path);
            recordFps = fps;
            recordMaxFrames = maxFrames;
        }

        void finishRecording();

        GLFWwindow* Window() { return this->window; }

        bool Paused() const { return this->in.Paused(); }

    private:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
        static constexpr float MIN_RADIUS_PX = 1.0f;

        GLFWwindow* window = nullptr;
        vk::raii::Context context;
        vk::raii::Instance instance = nullptr;
        vk::raii::SurfaceKHR surface = nullptr;

        camera         cam;
        interactions   in;
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
        uint32_t                             frameIndex = 0;
        bool                                 framebufferResized = false;

        std::vector<const char*> deviceExtensions = { vk::KHRSwapchainExtensionName };
        std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
        #ifdef DEBUG
        static constexpr bool enableValidationLayers = true;
        #else
        static constexpr bool enableValidationLayers = false;
        #endif
        vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;


        vk::raii::DescriptorPool descriptorPool = nullptr;
        vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
        vk::raii::DescriptorSets descriptorSets = nullptr;

        bool glfwInitialized = false;
        bool deviceInitialized = false;

        std::string recordPath;
        uint32_t recordFps = 60;
        size_t recordMaxFrames = 0;
        bool recordSizeWarned = false;
        FrameCapture capture;
        VideoEncoder encoder;

        inline bool recording() const noexcept { return !recordPath.empty(); }
        bool recordLimitReached() const noexcept { return recordMaxFrames != 0 && capture.captured() >= recordMaxFrames; }
        void start_recording();

        void recreate_swapchain();

        void init_window(size_t width, size_t height, bool headless);
        void vulkan_instance();
        void vulkan_surface();
        void vulkan_graphics_pipeline();
        void vulkan_command_pool();
        void vulkan_command_buffer();
        void vulkan_sync_objects();
        void vulkan_init_descriptors();
        void vulkan_write_descriptors(size_t i, vk::Buffer state, vk::Buffer masses);

        void vulkan_record_command_buffer(uint32_t imageIndex, simulation& sim, float ft);
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


        static uint32_t chooseSwapMinImageCount(const vk::SurfaceCapabilitiesKHR& capabilities);
        static uint32_t findQueueFamilies(vk::raii::PhysicalDevice physicalDevice);
        static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
        static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

        static const unsigned char shader_bytes[];
        static const size_t shader_size;
};
