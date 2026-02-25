#pragma once
#include <chrono>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <ranges>
#include <algorithm>

#include "../dependencies/dependencies.hpp"
#include "../definitions/definitions.hpp"
#include "../simulation/simulation.hpp"

struct renderer {
    public:

    std::chrono::nanoseconds render(const simulation& sim);
    void cleanup();

    int init(size_t n);
    bool should_close() { return glfwWindowShouldClose(window); }

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
    std::vector<vk::raii::ImageView> swapChainImageViews;

    std::vector<CircleData> circles;

    int init_window();
    int vulkan_instance();
    int vulkan_surface();
    int vulkan_physicaldevice();
    int vulkan_device();
    int vulkan_swapchain();
    int vulkan_image_views();
    int vulkan_graphics_pipeline();

    uint32_t findQueueFamilies(vk::raii::PhysicalDevice physicalDevice);
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats);
    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& presentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
    

    static const size_t width_ = 800;
    static const size_t height_ = 800;
};
