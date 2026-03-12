#pragma once
#include "../../definitions/graphics.hpp" // IWYU pragma: keep
#include "../physicaldevice/physicaldevice.hpp"
#include "../logicaldevice/logicaldevice.hpp"

struct swapchain {
    public:

    void init(const physicaldevice& pdevice, const logicaldevice& ldevice, const vk::raii::SurfaceKHR& surface, GLFWwindow* window);
    void recreate(const physicaldevice& pdevice, const logicaldevice& ldevice, const vk::raii::SurfaceKHR& surface, GLFWwindow* window);

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
