#pragma once
#include <chrono>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "../dependencies/dependencies.hpp"
#include "../definitions/definitions.hpp"
#include "../simulation/simulation.hpp"

struct renderer {
    public:

    // default constructor
    renderer() noexcept {}
    
    // move constructor
    renderer(renderer&& other) noexcept {
        std::cout << "move construct" << std::endl;
    }

    // copy constructor
    renderer(const renderer& other) noexcept {
        std::cout << "copy construct" << std::endl;
    }

    // deconstructor
    ~renderer() {
        cleanup();
    }

    // move operator
    renderer& operator = (renderer&& other) noexcept {
        std::memcpy(this, &other, sizeof(renderer));
        std::memset(&other, 0, sizeof(other));
        return *this;
    }

    // copy operator
    renderer& operator = (const renderer& other) noexcept {
        std::cout << "copy operator" << std::endl;
        return *this;
    }

    std::chrono::nanoseconds render(const simulation& sim);
    void cleanup();

    int init_vulkan(GLFWwindow* window, size_t n);

    private:
    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;

    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline circlePipeline = VK_NULL_HANDLE;

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkBuffer storageBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
    VkDeviceMemory storageBufferMemory = VK_NULL_HANDLE;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

    std::vector<CircleData> circles;

    int create_buffers(size_t n);
    int create_instance();
    int create_surface(GLFWwindow* window);
    int create_device();
    int create_pipeline();
    int set_descriptors(size_t n);
};
