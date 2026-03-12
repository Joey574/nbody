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
    
    inline auto& SwapChain() { return swapChain; }
    inline auto& Images() { return swapChainImages; }
    inline auto& SurfaceFormat() { return swapChainSurfaceFormat; }
    inline auto& Extent() { return swapChainExtent; }
    inline auto& ImageViews() { return swapChainImageViews; }

    inline const auto& SwapChain() const { return swapChain; }
    inline const auto& Images() const { return swapChainImages; }
    inline const auto& SurfaceFormat() const { return swapChainSurfaceFormat; }
    inline const auto& Extent() const { return swapChainExtent; }
    inline const auto& ImageViews() const { return swapChainImageViews; }

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
