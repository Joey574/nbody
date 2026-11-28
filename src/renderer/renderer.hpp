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
        ~renderer() { Cleanup(); }

        void Cleanup();
        void WaitIdle();

        void ReleaseSimulationReferences();

        LogicalDevice& GetLogicalDevice() noexcept { return ldevice; }
        PhysicalDevice& GetPhysicalDevice() noexcept { return pdevice; }

        std::chrono::nanoseconds Render(simulation& sim, float dt, float ft, bool paused);

        void InitVulkan(const cliargs& cli);
        void initRendering(const simulation& sim);

        bool ShouldClose() { return window == nullptr || glfwWindowShouldClose(window) || RecordLimitReached(); }

        void EnableRecording(const std::string& path, uint32_t fps, size_t maxFrames) {
            recordPath = path;
            recordFps = fps;
            recordMaxFrames = maxFrames;
        }

        inline void RegisterKey(int key, interactions::KeyCallback callback) {
            in.Register(key, callback);
        }

        void FinishRecording();

        GLFWwindow* Window() { return this->window; }

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

        inline bool Recording() const noexcept { return !recordPath.empty(); }
        bool RecordLimitReached() const noexcept { return recordMaxFrames != 0 && capture.captured() >= recordMaxFrames; }
        void StartRecording();

        void RecreateSwapchain();

        void InitWindow(size_t width, size_t height, bool headless);
        void InitInstance();
        void InitSurface();
        void GraphicsPipeline();
        void CommandPool();
        void InitCommandBuffer();
        void SyncObjects();
        void InitDescriptors();
        void WriteDescriptors(size_t i, vk::Buffer state, vk::Buffer masses);

        void RecordCommandBuffers(uint32_t imageIndex, simulation& sim, float ft, bool paused);
        void TransitionImageLayout(
            uint32_t imageIndex,
            vk::ImageLayout oldLayout,
            vk::ImageLayout newLayout,
            vk::AccessFlags2 srcAccessMask,
            vk::AccessFlags2 dstAccessMask,
            vk::PipelineStageFlags2 srcStageMask,
            vk::PipelineStageFlags2 dstStageMask
        );
        std::vector<const char*> GetRequiredInstanceExtensions();


        static uint32_t ChooseSwapMinImageCount(const vk::SurfaceCapabilitiesKHR& capabilities);
        static uint32_t FindQueueFamilies(vk::raii::PhysicalDevice physicalDevice);
        static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
        static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

        static const unsigned char shaderBytes[];
        static const size_t shaderSize;
};
