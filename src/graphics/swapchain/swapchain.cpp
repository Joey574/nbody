#include "swapchain.hpp"

void swapchain::init(const physicaldevice& pdevice, const logicaldevice& ldevice, const vk::raii::SurfaceKHR& surface, GLFWwindow* window) {
    const auto& pd = pdevice.get();
    const auto& ld = ldevice.getDevice();

    swapChainImageViews.clear();
    swapChain = nullptr;
    
    auto sfCapabilities = pd.getSurfaceCapabilitiesKHR(surface);
    swapChainExtent = chooseSwapExtent(sfCapabilities, window);
    swapChainSurfaceFormat = chooseSwapSurfaceFormat(pd.getSurfaceFormatsKHR(surface));

    vk::SwapchainCreateInfoKHR swapChainCreateInfo {
        .surface = surface,
        .minImageCount = chooseSwapMinImageCount(sfCapabilities),
        .imageFormat = swapChainSurfaceFormat.format,
        .imageColorSpace = swapChainSurfaceFormat.colorSpace,
        .imageExtent = swapChainExtent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .preTransform = sfCapabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = chooseSwapPresentMode(pd.getSurfacePresentModesKHR(surface)),
        .clipped = true
    };

    swapChain = vk::raii::SwapchainKHR(ld, swapChainCreateInfo);
    swapChainImages = swapChain.getImages();

    image_views(ld);
}

void swapchain::recreate(const physicaldevice& pdevice, const logicaldevice& ldevice, const vk::raii::SurfaceKHR& surface, GLFWwindow* window) {
    const auto& ld = ldevice.getDevice();

    int w, h = 0;
    glfwGetFramebufferSize(window, &w, &h);
    while (w == 0 || h == 0) {
        glfwGetFramebufferSize(window, &w, &h);
        glfwWaitEvents();
    }

    ld.waitIdle();
    init(pdevice, ldevice, surface, window);
}

void swapchain::image_views(const vk::raii::Device& ldevice) {
    swapChainImageViews.clear();
    vk::ImageViewCreateInfo imageViewCreateInfo {
        .viewType = vk::ImageViewType::e2D,
        .format = swapChainSurfaceFormat.format,
        .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
    };

    for (const auto& image : swapChainImages) {
        imageViewCreateInfo.image = image;
        swapChainImageViews.emplace_back(ldevice, imageViewCreateInfo);
    }
}

uint32_t swapchain::chooseSwapMinImageCount(const vk::SurfaceCapabilitiesKHR& capabilities) {
    auto minImageCount = std::max(3u, capabilities.minImageCount);
    if (0 < capabilities.maxImageCount && capabilities.maxImageCount < minImageCount) {
        minImageCount = capabilities.maxImageCount;
    }

    return minImageCount;
}

vk::PresentModeKHR swapchain::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& modes) {
    bool hasImmediate = false;
    bool hasMailbox = false;

    for (const auto& m : modes) {
        hasImmediate |= (m == vk::PresentModeKHR::eImmediate);
        hasMailbox   |= (m == vk::PresentModeKHR::eMailbox);
    }

    return hasImmediate ? vk::PresentModeKHR::eImmediate :
           hasMailbox   ? vk::PresentModeKHR::eMailbox   :
           vk::PresentModeKHR::eFifo;
}

vk::SurfaceFormatKHR swapchain::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats) {
    for (const auto& f : formats) {
        if (f.format == vk::Format::eB8G8R8A8Srgb && f.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return f;
        }
    }

    return formats[0];
}

vk::Extent2D swapchain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);

    return {
        std::clamp<uint32_t>(w, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp<uint32_t>(h, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
    };
}
