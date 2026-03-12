#pragma once
#include "../../definitions/graphics.hpp" // IWYU pragma: keep
#include "../PhysicalDevice/PhysicalDevice.hpp"
#include "../LogicalDevice/LogicalDevice.hpp"

struct Swapchain {
    public:

    void init(const PhysicalDevice& pdevice, const LogicalDevice& ldevice, const vk::raii::SurfaceKHR& surface, GLFWwindow* window);
    void recreate(const PhysicalDevice& pdevice, const LogicalDevice& ldevice, const vk::raii::SurfaceKHR& surface, GLFWwindow* window);

    inline operator vk::raii::SwapchainKHR&() { return swapChain; }
    inline operator const vk::raii::SwapchainKHR&() const { return swapChain; }
    
    inline auto& getSwapChain() { return swapChain; }
    inline auto& getImages() { return swapChainImages; }
    inline auto& getSurfaceFormat() { return swapChainSurfaceFormat; }
    inline auto& getExtent() { return swapChainExtent; }
    inline auto& getImageViews() { return swapChainImageViews; }

    inline const auto& getSwapChain() const { return swapChain; }
    inline const auto& getImages() const { return swapChainImages; }
    inline const auto& getSurfaceFormat() const { return swapChainSurfaceFormat; }
    inline const auto& getExtent() const { return swapChainExtent; }
    inline const auto& getImageViews() const { return swapChainImageViews; }

    private:

    vk::raii::SwapchainKHR           swapChain = nullptr;
    std::vector<vk::Image>           swapChainImages;
    vk::SurfaceFormatKHR             swapChainSurfaceFormat;
    vk::Extent2D                     swapChainExtent;
    std::vector<vk::raii::ImageView> swapChainImageViews;

    void image_views(const vk::raii::Device& ldevice);

    static uint32_t chooseSwapMinImageCount(const vk::SurfaceCapabilitiesKHR& capabilities);
    static vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& modes);
    static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats);
    static vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);
};
