/* 
Author: Joey Soroka
Purpose: Contains vulkan initilization utilities for the renderer struct
Comments: Most of this code is ripped from https://docs.vulkan.org/tutorial/latest/00_Introduction.html
*/

#include "renderer.hpp"
#include <stdexcept>

void renderer::init(const data& data, const std::string& exePath) {
    auto shaderPath = exePath.substr(0, exePath.find_last_of('/')) + "/tri.spv";

    init_window();
    vulkan_instance();
    vulkan_surface();
    pdevice.init(instance);
    ldevice.init(pdevice, surface);
    swapchain.init(pdevice, ldevice, surface, window);
    vulkan_graphics_pipeline(shaderPath);
    vulkan_command_pool();
    vulkan_vertex_buffer(data);
    // TODO : reimplment below line eventually
    //command.init(ldevice, MAX_FRAMES_IN_FLIGHT);
    vulkan_command_buffer();
    vulkan_sync_objects();
}

void renderer::init_window() {
    if (!glfwInit()) { 
        throw std::runtime_error("failed to init glfw");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(width_, height_, "nbody", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("failed to create glfw window");
    }

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

    glfwShowWindow(window);
}

void renderer::vulkan_instance() {
    constexpr vk::ApplicationInfo appInfo {
        .pApplicationName = "nbody",
        .applicationVersion = VK_MAKE_VERSION( 1, 0, 0 ),
        .pEngineName        = "No Engine",
        .engineVersion      = VK_MAKE_VERSION( 1, 0, 0 ),
        .apiVersion         = vk::ApiVersion14
    };

    auto requiredExtensions = getRequiredInstanceExtensions();

    vk::InstanceCreateInfo createInfo {
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
        .ppEnabledExtensionNames = requiredExtensions.data()
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

void renderer::vulkan_graphics_pipeline(const std::string& shaderPath) {
    vk::raii::ShaderModule shaderModule = createShaderModule(
        std::vector<char>(renderer::shader_bytes, renderer::shader_bytes + renderer::shader_size)
    );

    vk::PipelineShaderStageCreateInfo vertShaderStageInfo {
        .stage  = vk::ShaderStageFlagBits::eVertex,
        .module = *shaderModule,
        .pName  = "vertMain"
    };

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo {
        .stage  = vk::ShaderStageFlagBits::eFragment,
        .module = *shaderModule,
        .pName  = "fragMain"
    };

    vk::PipelineShaderStageCreateInfo shaderStages[] = {
        vertShaderStageInfo,
        fragShaderStageInfo
    };

    auto bindingDescription    = vertex::getBindingDescription();
    auto attributeDescriptions = vertex::getAttributeDescriptions();

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo {
        .vertexBindingDescriptionCount   = 1,
        .pVertexBindingDescriptions      = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions    = attributeDescriptions.data()
    };

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly {
        .topology = vk::PrimitiveTopology::eTriangleList
    };

    std::vector<vk::DynamicState> dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamicState {
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates    = dynamicStates.data()
    };

    vk::PipelineViewportStateCreateInfo viewportState {
        .viewportCount = 1,
        .scissorCount  = 1
    };

    vk::PipelineRasterizationStateCreateInfo rasterizer {
        .depthClampEnable        = vk::False,
        .rasterizerDiscardEnable = vk::False,
        .polygonMode             = vk::PolygonMode::eFill,
        .cullMode                = vk::CullModeFlagBits::eBack,
        .frontFace               = vk::FrontFace::eClockwise,
        .depthBiasEnable         = vk::False,
        .depthBiasSlopeFactor    = 1.0f,
        .lineWidth               = 1.0f
    };

    vk::PipelineMultisampleStateCreateInfo multisampling {
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable  = vk::False
    };

    vk::PipelineColorBlendAttachmentState colorBlendAttachment {
        .blendEnable    = vk::False,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    };

    vk::PipelineColorBlendStateCreateInfo colorBlending {
        .logicOpEnable   = vk::False,
        .logicOp         = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments    = &colorBlendAttachment
    };

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo {
        .setLayoutCount         = 0,
        .pushConstantRangeCount = 0
    };

    pipelineLayout = vk::raii::PipelineLayout(ldevice, pipelineLayoutInfo);

    vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo {
        .colorAttachmentCount    = 1,
        .pColorAttachmentFormats = &swapchain.SurfaceFormat().format
    };

    vk::GraphicsPipelineCreateInfo pipelineInfo {
        .pNext               = &pipelineRenderingCreateInfo,
        .stageCount          = 2,
        .pStages             = shaderStages,
        .pVertexInputState   = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState      = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState   = &multisampling,
        .pColorBlendState    = &colorBlending,
        .pDynamicState       = &dynamicState,
        .layout              = pipelineLayout,
        .renderPass          = nullptr
    };

    pipeline = vk::raii::Pipeline(ldevice, nullptr, pipelineInfo);
}

void renderer::vulkan_command_pool() {
    vk::CommandPoolCreateInfo poolInfo {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = ldevice.QueueIdx()
    };

    commandPool = vk::raii::CommandPool(ldevice, poolInfo);
}

void renderer::vulkan_command_buffer() {
    commandBuffers.clear();

    vk::CommandBufferAllocateInfo allocInfo {
        .commandPool = commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT
    };

    commandBuffers = vk::raii::CommandBuffers(ldevice, allocInfo);
}

void renderer::vulkan_sync_objects() {
    assert(presentCompleteSemaphores.empty() && renderFinishedSemaphores.empty() && inFlightFences.empty());

    for (size_t i = 0; i < swapchain.Images().size(); i++) {
        renderFinishedSemaphores.emplace_back(ldevice, vk::SemaphoreCreateInfo());
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        presentCompleteSemaphores.emplace_back(ldevice, vk::SemaphoreCreateInfo());
        inFlightFences.emplace_back(ldevice, vk::FenceCreateInfo{
            .flags = vk::FenceCreateFlagBits::eSignaled
        });
    }
}

void renderer::vulkan_record_command_buffer(uint32_t imageIndex) {
    auto& commandBuffer = commandBuffers[frameIndex];
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
        .imageView = swapchain.ImageViews()[imageIndex],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearColor
    };

    vk::RenderingInfo renderingInfo = {
        .renderArea = { 
            .offset = { 0, 0}, 
            .extent = swapchain.Extent() 
        },
        .layerCount           = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments    = &attachmentInfo
    };

    commandBuffer.beginRendering(renderingInfo);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
    commandBuffer.bindVertexBuffers(0, *vertexBuffer, {0});
    commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapchain.Extent().width), static_cast<float>(swapchain.Extent().height), 0.0f, 1.0f));
    commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapchain.Extent()));
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
    auto& commandBuffer = commandBuffers[frameIndex];
    vk::ImageMemoryBarrier2 barrier = {
        .srcStageMask = srcStageMask,
        .srcAccessMask = srcAccessMask,
        .dstStageMask = dstStageMask,
        .dstAccessMask = dstAccessMask,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = swapchain.Images()[imageIndex],
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

void renderer::vulkan_vertex_buffer(const data& data) {
    vk::BufferCreateInfo bufferInfo {
        .size = sizeof(vertices[0]) * vertices.size(),
        .usage = vk::BufferUsageFlagBits::eVertexBuffer,
        .sharingMode = vk::SharingMode::eExclusive
    };

    vertexBuffer = vk::raii::Buffer(ldevice, bufferInfo);

    vk::MemoryRequirements memRequirements = vertexBuffer.getMemoryRequirements();

    vk::MemoryAllocateInfo memAllocateInfo {
        .allocationSize = memRequirements.size, 
        .memoryTypeIndex = find_memory_type(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
    };

    vertexBufferMemory = vk::raii::DeviceMemory(ldevice, memAllocateInfo);
    vertexBuffer.bindMemory(*vertexBufferMemory, 0);

    void* mapped = vertexBufferMemory.mapMemory(0, bufferInfo.size);
    memcpy(mapped, vertices.data(), bufferInfo.size);
    vertexBufferMemory.unmapMemory();
}

void renderer::vulkan_update_vertex_buffer(const data& data) {
}

std::chrono::nanoseconds renderer::render(const data& data) {
    auto s = std::chrono::high_resolution_clock::now();

    vulkan_update_vertex_buffer(data);

    auto fenceResult = ldevice.Device().waitForFences(*inFlightFences[frameIndex], vk::True, UINT64_MAX);
    if (fenceResult != vk::Result::eSuccess) {
        throw std::runtime_error("failed to wait for fence");
    }

    auto [result, imageIndex] = swapchain.SwapChain().acquireNextImage(UINT64_MAX, *presentCompleteSemaphores[frameIndex], nullptr);
    if (result == vk::Result::eErrorOutOfDateKHR) {
        swapchain.recreate(pdevice, ldevice, surface, window);
        return std::chrono::high_resolution_clock::now() - s;
    } else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("failed to acquire swap chain image");
    }

    ldevice.Device().resetFences(*inFlightFences[frameIndex]);

    commandBuffers[frameIndex].reset();
    vulkan_record_command_buffer(imageIndex);

    vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    const vk::SubmitInfo submitInfo {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*presentCompleteSemaphores[frameIndex],
        .pWaitDstStageMask = &waitDestinationStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &*commandBuffers[frameIndex],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &*renderFinishedSemaphores[frameIndex]
    };

    ldevice.Queue().submit(submitInfo, *inFlightFences[frameIndex]);

    const vk::PresentInfoKHR presentInfoKHR {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*renderFinishedSemaphores[frameIndex],
        .swapchainCount = 1,
        .pSwapchains = &*swapchain.SwapChain(),
        .pImageIndices = &imageIndex
    };

    result = ldevice.Queue().presentKHR(presentInfoKHR);
    if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR || framebufferResized) {
        framebufferResized = false;
        swapchain.recreate(pdevice, ldevice, surface, window);
    }

    frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
    return std::chrono::high_resolution_clock::now() - s;
}

void renderer::cleanup() {
    ldevice.Device().waitIdle();

    if (window) { glfwDestroyWindow(window); }
    glfwTerminate();
}
