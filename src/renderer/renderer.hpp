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

    std::vector<CircleData> circles;

    int init_window();
    int vulkan_device();
    int vulkan_surface();
    int vulkan_instance();
    int vulkan_physicaldevice();

    uint32_t findQueueFamilies(vk::raii::PhysicalDevice physicalDevice);

    static const size_t width_ = 800;
    static const size_t height_ = 800;
};
