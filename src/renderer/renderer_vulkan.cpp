/* 
Author: Joey Soroka
Updated: 2/26/26
Purpose: Contains vulkan initilization utilities for the renderer struct
Comments: Most of this code is ripped from https://docs.vulkan.org/tutorial/latest/00_Introduction.html
*/

#include "renderer.hpp"

void renderer::init(size_t n) {
    init_window();
    vulkan_instance();
    vulkan_surface();
    vulkan_physicaldevice();
    vulkan_device();
    vulkan_swapchain();
    vulkan_image_views();
    vulkan_graphics_pipeline();
    vulkan_command_pool();
    vulkan_command_buffer();
}

void renderer::init_window() {
    if (!glfwInit()) { 
        throw std::runtime_error("failed to init glfw");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(width_, height_, "nbody", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("failed to create glfw window");
    }

    glfwShowWindow(window);
}

void renderer::vulkan_instance() {
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
}

void renderer::vulkan_surface() {
    VkSurfaceKHR _surface;
    if (glfwCreateWindowSurface(*instance, window, nullptr, &_surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create glfw window surface");
    }

    surface = vk::raii::SurfaceKHR(instance, _surface);
}

void renderer::vulkan_physicaldevice() {
    auto devices = instance.enumeratePhysicalDevices();
    if (devices.empty()) {
        throw std::runtime_error("no devices :/");
    }

    physicalDevice = devices[0];
}

void renderer::vulkan_device() {
    // find the index of the first queue family that supports graphics
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
    

    // get the first index into queueFamilyProperties which supports graphics
    auto graphicsQueueFamilyProperty = std::ranges::find_if( queueFamilyProperties, []( auto const & qfp )
                    { return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0); } );

    graphicsIndex = static_cast<uint32_t>( std::distance( queueFamilyProperties.begin(), graphicsQueueFamilyProperty ) );

    // determine a queueFamilyIndex that supports present
    // first check if the graphicsIndex is good enough
    presentIndex = physicalDevice.getSurfaceSupportKHR( graphicsIndex, *surface )
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
        throw std::runtime_error("failed to find device");
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
}

void renderer::vulkan_swapchain() {
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
}

void renderer::vulkan_image_views() {
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
}

void renderer::vulkan_graphics_pipeline() {
    auto shaderCode = readFile("slang.spv");
    vk::raii::ShaderModule shaderModule = createShaderModule(shaderCode);

    vk::PipelineShaderStageCreateInfo vertShaderStageInfo {
        .stage = vk::ShaderStageFlagBits::eVertex,
        .module = shaderModule,
        .pName = "vertMain"
    };

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo {
        .stage = vk::ShaderStageFlagBits::eFragment,
        .module = shaderModule,
        .pName = "fragMain"
    };

    vk::PipelineShaderStageCreateInfo shaderStages[] = {
        vertShaderStageInfo,
        fragShaderStageInfo
    };

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly {
        .topology = vk::PrimitiveTopology::eTriangleList
    };

    vk::Viewport viewport {
        0.0f,
        0.0f,
        static_cast<float>(swapChainExtent.width),
        static_cast<float>(swapChainExtent.height),
        0.0f,
        1.0f
    };

    std::vector<vk::DynamicState> dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamicState {
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()
    };

    constexpr vk::PipelineViewportStateCreateInfo viewportState {
        .viewportCount = 1,
        .scissorCount = 1
    };

    constexpr vk::PipelineRasterizationStateCreateInfo rasterizer {
        .depthClampEnable = vk::False,
        .rasterizerDiscardEnable = vk::False,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eClockwise,
        .depthBiasEnable = vk::False,
        .depthBiasSlopeFactor = 1.0f,
        .lineWidth = 1.0f
    };

    constexpr vk::PipelineMultisampleStateCreateInfo multisampling {
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = vk::False
    };

    constexpr vk::PipelineColorBlendAttachmentState colorBlendAttachment {
        .blendEnable = vk::False,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    };

    vk::PipelineColorBlendStateCreateInfo colorBlending {
        .logicOpEnable = vk::False,
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment
    };

    constexpr vk::PipelineLayoutCreateInfo pipelineLayoutInfo {
        .setLayoutCount = 0,
        .pushConstantRangeCount = 0
    };

    pipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutInfo);

    vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo {
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &swapChainImageFormat
    };

    vk::GraphicsPipelineCreateInfo pipelineInfo {
        .pNext = &pipelineRenderingCreateInfo,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = pipelineLayout,
        .renderPass = nullptr
    };

    graphicsPipeline = vk::raii::Pipeline(device, nullptr, pipelineInfo);
}

void renderer::vulkan_command_pool() {
    vk::CommandPoolCreateInfo poolInfo {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = graphicsIndex
    };

    commandPool = vk::raii::CommandPool(device, poolInfo);
}

void renderer::vulkan_command_buffer() {
    vk::CommandBufferAllocateInfo allocInfo {
        .commandPool = commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };

    commandBuffer = std::move(vk::raii::CommandBuffers(device, allocInfo).front());
}

void renderer::vulkan_record_command_buffer(uint32_t imageIndex) {
    commandBuffer.begin({});

    transition_image_layout(
        imageIndex,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        {},
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput
    );

    vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
    vk::RenderingAttachmentInfo attachmentInfo = {
        .imageView = swapChainImageViews[imageIndex],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearColor
    };

    vk::RenderingInfo renderingInfo = {
        .renderArea = { 
            .offset = { 0, 0}, 
            .extent = swapChainExtent 
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentInfo
    };

    commandBuffer.beginRendering(renderingInfo);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
    commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 0.0f, 1.0f));
    commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapChainExtent));
    commandBuffer.draw(3, 1, 0, 0);
    commandBuffer.endRendering();

    transition_image_layout(
        imageIndex,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        {},
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eBottomOfPipe
    );

    commandBuffer.end();
}

void renderer::transition_image_layout(
    uint32_t imageIndex,
    vk::ImageLayout oldLayout,
    vk::ImageLayout newLayout,
    vk::AccessFlags2 srcAccessMask,
    vk::AccessFlags2 dstAccessMask,
    vk::PipelineStageFlags2 srcStageMask,
    vk::PipelineStageFlags2 dstStageMask    
) {
    vk::ImageMemoryBarrier2 barrier = {
        .srcStageMask = srcStageMask,
        .srcAccessMask = srcAccessMask,
        .dstStageMask = dstStageMask,
        .dstAccessMask = dstAccessMask,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = swapChainImages[imageIndex],
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    vk::DependencyInfo dependencyInfo = {
        .dependencyFlags = {},
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier
    };

    commandBuffer.pipelineBarrier2(dependencyInfo);
}

[[nodiscard]] vk::raii::ShaderModule renderer::createShaderModule(const std::vector<char>& code) {
    vk::ShaderModuleCreateInfo createInfo {
        .codeSize = code.size() * sizeof(char),
        .pCode = reinterpret_cast<const uint32_t*>(code.data())
    };

    vk::raii::ShaderModule shaderModule{device, createInfo};
    return shaderModule;
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

std::chrono::nanoseconds renderer::render(const data& data) {
    auto s = std::chrono::high_resolution_clock::now();
    return std::chrono::high_resolution_clock::now() - s;
}

void renderer::cleanup() {
    if (window) { glfwDestroyWindow(window); }
    glfwTerminate();
}
