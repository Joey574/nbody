module;
#include <chrono>
#include <string>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

export module renderer;
import simulation;

export struct renderer {
    public:

    std::chrono::nanoseconds render(const simulation& sim);
    void cleanup();

    int init(size_t n);
    bool should_close() { return glfwWindowShouldClose(window); }
    void poll_events() { glfwPollEvents(); }

    private:
    GLFWwindow* window = nullptr;

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

    int init_window();
    int vulkan_instance();
    int vulkan_surface();
    int vulkan_physicaldevice();
    int vulkan_device();
    int vulkan_swapchain();
    int vulkan_image_views();
    int vulkan_graphics_pipeline();

    std::vector<char> readFile(const std::string& path);
    [[nodiscard]] vk::raii::ShaderModule createShaderModule(const std::vector<char>& code);

    uint32_t findQueueFamilies(vk::raii::PhysicalDevice physicalDevice);
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats);
    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& presentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
    

    static const size_t width_ = 800;
    static const size_t height_ = 800;
};
