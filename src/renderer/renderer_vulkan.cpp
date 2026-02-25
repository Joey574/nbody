/* 
Author: Joey Soroka
Updated: 2/23/26
Purpose: Contains vulkan initilization utilities for the renderer struct
Comments: Most of this code is ripped from https://docs.vulkan.org/tutorial/latest/00_Introduction.html
*/

#include "renderer.hpp"

int renderer::init(size_t n) {
    if (init_window()) { 
        std::__throw_runtime_error("failed to init window");
        return 1;
    }

    if (vulkan_instance()) { 
        std::__throw_runtime_error("failed to init vulkan instance");
        return 1;
    }

    if (vulkan_surface()) { 
        std::__throw_runtime_error("failed to init vulkan surface");
        return 1;
    }

    if (vulkan_physicaldevice()) { 
        std::__throw_runtime_error("failed to init vulkan physical device");
        return 1;
    }

    if (vulkan_device()) { 
        std::__throw_runtime_error("failed to init vulkan device");
        return 1;
    }

    if (vulkan_swapchain()) {
        std::__throw_runtime_error("failed to init swapchain");
        return 1;
    }

    return 0;
}

int renderer::init_window() {
    if (!glfwInit()) { return 1; }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(width_, height_, "nbody simulation", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }

    return 0;
}

int renderer::vulkan_instance() {
    constexpr vk::ApplicationInfo appInfo{
        .pApplicationName = "nbody",
        .applicationVersion = VK_MAKE_VERSION( 1, 0, 0 ),
        .pEngineName        = "No Engine",
        .engineVersion      = VK_MAKE_VERSION( 1, 0, 0 ),
        .apiVersion         = vk::ApiVersion13
    };

    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    
    vk::InstanceCreateInfo createInfo{
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = glfwExtensionCount,
        .ppEnabledExtensionNames = glfwExtensions
    };

    instance = vk::raii::Instance(context, createInfo);
    return 0;
}

int renderer::vulkan_surface() {
    VkSurfaceKHR _surface;
    if (glfwCreateWindowSurface(*instance, window, nullptr, &_surface) != VK_SUCCESS) {
        return 1;
    }
    
    surface = vk::raii::SurfaceKHR(instance, _surface);
    return 0;
}

int renderer::vulkan_physicaldevice() {
    auto devices = instance.enumeratePhysicalDevices();
    if (devices.empty()) {
        return 1;
    }

    physicalDevice = devices[0];
    return 0;
}

int renderer::vulkan_device() {
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
    uint32_t graphicsIndex = findQueueFamilies(physicalDevice);
    uint32_t presentIndex = physicalDevice.getSurfaceSupportKHR(graphicsIndex, *surface) ? graphicsIndex : static_cast<uint32_t>(queueFamilyProperties.size());

    float queuePriority = 0.5f;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo { .queueFamilyIndex = graphicsIndex, .queueCount = 1, .pQueuePriorities = &queuePriority };
    return 0;

    // Create a chain of feature structures
    vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain = {
        {},                               // vk::PhysicalDeviceFeatures2 (empty for now)
        {.dynamicRendering = true },      // Enable dynamic rendering from Vulkan 1.3
        {.extendedDynamicState = true }   // Enable extended dynamic state from the extension
    };

    std::vector<const char*> deviceExtensions = {
        vk::KHRSwapchainExtensionName
    };

    vk::DeviceCreateInfo deviceCreateInfo {
        .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &deviceQueueCreateInfo,
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data()
    };

    device = vk::raii::Device( physicalDevice, deviceCreateInfo );
    graphicsQueue = vk::raii::Queue( device, graphicsIndex, 0 );
    presentQueue = vk::raii::Queue( device, presentIndex, 0 );
}

int renderer::vulkan_swapchain() {
    auto surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
    swapChainSurfaceFormat = chooseSwapSurfaceFormat(physicalDevice.getSurfaceFormatsKHR(*surface));
    swapChainExtent = chooseSwapExtent(surfaceCapabilities);
    auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
    minImageCount = (surfaceCapabilities.maxImageCount > 0 && minImageCount > surfaceCapabilities.maxImageCount) ? surfaceCapabilities.maxImageCount : minImageCount;

    vk::SwapchainCreateInfoKHR swapChainCreateInfo {
        .flags = vk::SwapchainCreateFlagsKHR(),
        .surface = *surface,
        .minImageCount = minImageCount,
        .imageFormat = swapChainSurfaceFormat.format,
        .imageColorSpace = swapChainSurfaceFormat.colorSpace,
        .imageExtent = swapChainExtent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .preTransform = surfaceCapabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = chooseSwapPresentMode(physicalDevice.getSurfacePresentModesKHR(*surface)),
        .clipped = true,
        .oldSwapchain = nullptr
    };

    uint32_t queueFamilyIndicies[] = {graphicsFamily, presentFamily};

    if (graphicsFamily != presentFamily) {
        swapChainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        swapChainCreateInfo.queueFamilyIndexCount = 2;
        swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        swapChainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
        swapChainCreateInfo.queueFamilyIndexCount = 0;
        swapChainCreateInfo.pQueueFamilyIndices = nullptr;
    }

    swapChainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
    swapChainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

    return 0;
}

int renderer::vulkan_image_views() {
    swapChainImageViews.clear();
    vk::ImageViewCreateInfo imageViewCreateInfo {
        .viewType = vk::ImageViewType::e2D,
        .format = swapChainImageFormat,
        .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
    };

    for (auto image : swapChainImageViews) {
        imageViewCreateInfo.image = image;
        swapChainImageViews.emplace_back(device, imageViewCreateInfo);
    }
}

int renderer::vulkan_graphics_pipeline() {

}

uint32_t renderer::findQueueFamilies(vk::raii::PhysicalDevice physicalDevice) {
    // find the index of the first queue family that supports graphics
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

    // get the first index into queueFamilyProperties which supports graphics
    auto graphicsQueueFamilyProperty =
      std::find_if( queueFamilyProperties.begin(),
                    queueFamilyProperties.end(),
                    []( vk::QueueFamilyProperties const & qfp ) { return qfp.queueFlags & vk::QueueFlagBits::eGraphics; } );

    return static_cast<uint32_t>( std::distance( queueFamilyProperties.begin(), graphicsQueueFamilyProperty ) );
}

vk::SurfaceFormatKHR renderer::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats) {
    return formats[0];
}

vk::PresentModeKHR renderer::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& presentModes) {
    for (const auto& mode : presentModes) {
        if (mode == vk::PresentModeKHR::eMailbox) {
            return mode;
        }
    }

    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D renderer::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);

    return {
        std::clamp<uint32_t>(w, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp<uint32_t>(h, capabilities.minImageExtent.height, capabilities.maxImageExtent.height),
    };
}

void renderer::cleanup() {
    if (window) { glfwDestroyWindow(window); }
    glfwTerminate();
}
