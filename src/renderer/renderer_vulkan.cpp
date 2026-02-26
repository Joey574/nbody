/* 
Author: Joey Soroka
Updated: 2/25/26
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

    if (vulkan_image_views()) {
        std::__throw_runtime_error("failed to init image views");
        return 1;

    }

    return 0;
}

int renderer::init_window() {
    if (!glfwInit()) { return 1; }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

    window = glfwCreateWindow(width_, height_, "nbody", nullptr, nullptr);
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
    // find the index of the first queue family that supports graphics
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
    

    // get the first index into queueFamilyProperties which supports graphics
    auto graphicsQueueFamilyProperty = std::ranges::find_if( queueFamilyProperties, []( auto const & qfp )
                    { return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0); } );

    auto graphicsIndex = static_cast<uint32_t>( std::distance( queueFamilyProperties.begin(), graphicsQueueFamilyProperty ) );

    // determine a queueFamilyIndex that supports present
    // first check if the graphicsIndex is good enough
    auto presentIndex = physicalDevice.getSurfaceSupportKHR( graphicsIndex, *surface )
                                       ? graphicsIndex
                                       : static_cast<uint32_t>( queueFamilyProperties.size() );
    if ( presentIndex == queueFamilyProperties.size() )
    {
        // the graphicsIndex doesn't support present -> look for another family index that supports both
        // graphics and present
        for ( size_t i = 0; i < queueFamilyProperties.size(); i++ )
        {
            if ( ( queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics ) &&
                 physicalDevice.getSurfaceSupportKHR( static_cast<uint32_t>( i ), *surface ) )
            {
                graphicsIndex = static_cast<uint32_t>( i );
                presentIndex  = graphicsIndex;
                break;
            }
        }
        if ( presentIndex == queueFamilyProperties.size() )
        {
            // there's nothing like a single family index that supports both graphics and present -> look for another
            // family index that supports present
            for ( size_t i = 0; i < queueFamilyProperties.size(); i++ )
            {
                if ( physicalDevice.getSurfaceSupportKHR( static_cast<uint32_t>( i ), *surface ) )
                {
                    presentIndex = static_cast<uint32_t>( i );
                    break;
                }
            }
        }
    }
    if ( ( graphicsIndex == queueFamilyProperties.size() ) || ( presentIndex == queueFamilyProperties.size() ) )
    {
        return 1;
    }

    // query for Vulkan 1.3 features
    auto features = physicalDevice.getFeatures2();
    vk::PhysicalDeviceVulkan13Features vulkan13Features;
    vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures;
    vulkan13Features.dynamicRendering = vk::True;
    extendedDynamicStateFeatures.extendedDynamicState = vk::True;
    vulkan13Features.pNext = &extendedDynamicStateFeatures;
    features.pNext = &vulkan13Features;
    
    // create a Device
    float                     queuePriority = 0.5f;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo { .queueFamilyIndex = graphicsIndex, .queueCount = 1, .pQueuePriorities = &queuePriority };
    vk::DeviceCreateInfo      deviceCreateInfo{ .pNext =  &features, .queueCreateInfoCount = 1, .pQueueCreateInfos = &deviceQueueCreateInfo };
    deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    device = vk::raii::Device( physicalDevice, deviceCreateInfo );
    graphicsQueue = vk::raii::Queue( device, graphicsIndex, 0 );
    presentQueue = vk::raii::Queue( device, presentIndex, 0 );

    return 0;
}

int renderer::vulkan_swapchain() {
    auto surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
    swapChainSurfaceFormat = chooseSwapSurfaceFormat(physicalDevice.getSurfaceFormatsKHR(*surface));
    swapChainExtent = chooseSwapExtent(surfaceCapabilities);
    presentMode = chooseSwapPresentMode(physicalDevice.getSurfacePresentModesKHR(*surface));
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
        .presentMode = presentMode,
        .clipped = true,
        .oldSwapchain = nullptr
    };

    // TODO : fix this
    //uint32_t graphicsFamily = findQueueFamilies(physicalDevice);
    //uint32_t presentFamily = physicalDevice.getQueueFamilyProperties();
    uint32_t graphicsFamily = 0;
    uint32_t presentFamily = 0;

    uint32_t queueFamilyIndices[] = {graphicsFamily, presentFamily};
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

    swapChainCreateInfo.presentMode = presentMode;
    swapChainCreateInfo.clipped = vk::True;
    swapChainCreateInfo.oldSwapchain = nullptr;

    swapChain = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
    swapChainImages = swapChain.getImages();
    swapChainImageFormat = swapChainCreateInfo.imageFormat;
    swapChainExtent = swapChainCreateInfo.imageExtent;
    return 0;
}

int renderer::vulkan_image_views() {
    swapChainImageViews.clear();
    vk::ImageViewCreateInfo imageViewCreateInfo {
        .viewType = vk::ImageViewType::e2D,
        .format = swapChainImageFormat,
        .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
    };

    for (auto image : swapChainImages) {
        imageViewCreateInfo.image = image;
        swapChainImageViews.emplace_back(device, imageViewCreateInfo);
    }

    return 0;
}

int renderer::vulkan_graphics_pipeline() {
    return 0;
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
    for (const auto& f : formats) {
        if (f.format == vk::Format::eB8G8R8A8Srgb && f.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return f;
        }
    }

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
