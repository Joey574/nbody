/*
Author: Joey Soroka
Purpose: Contains vulkan initilization utilities for the renderer struct
Comments: Most of this code is ripped from https://docs.vulkan.org/tutorial/latest/00_Introduction.html
*/

#include "graphics/FrameCapture/FrameCapture.hpp"
#include "renderer/renderer.hpp"
#include "graphics/vutil.hpp"

#include <chrono>
#include <cstdio>
#include <stdexcept>

void renderer::initVulkan(const cliargs& cli) {
    if (volkInitialize() != VK_SUCCESS) {
        throw std::runtime_error("no vulkan loader on this machine");
    }

    cam = camera(cli.x, cli.y, cli.zoom);
    in = interactions(cli.pause);

    init_window(cli.width, cli.height, cli.headless);
    vulkan_instance();
    volkLoadInstance(*instance);
    vulkan_surface();
    pdevice.init(instance);
    ldevice.init(pdevice, surface);
    volkLoadDevice(*ldevice.Device());
    deviceInitialized = true;

    if (recording()) swapchain.setExtraUsage(vk::ImageUsageFlagBits::eTransferSrc);

    swapchain.init(pdevice, ldevice, surface, window);
    vulkan_init_descriptors();
    vulkan_graphics_pipeline();
    vulkan_command_pool();
}

void renderer::initRendering(const simulation& sim) {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (!sim.isGPU()) {
            frames[i].init(sim.get_data(), ldevice, pdevice);
        }

        uboBuffers[i].init(ldevice, pdevice);
    }

    vulkan_command_buffer();
    vulkan_sync_objects();
}

void renderer::init_window(size_t width, size_t height, bool headless) {
    if (!glfwInit()) throw std::runtime_error("failed to init glfw");
    if (width % 2 || height % 2) throw std::invalid_argument("width & height must both be even");

    this->glfwInitialized = true;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, recording() ? GLFW_FALSE : GLFW_TRUE);

    window = glfwCreateWindow(width, height, "nbody", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        this->glfwInitialized = false;
        throw std::runtime_error("failed to create glfw window");
    }

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

    glfwSetScrollCallback(window, [](GLFWwindow* w, double x, double y){
       auto r = static_cast<renderer*>(glfwGetWindowUserPointer(w));
       r->cam.onScroll(y);
    });

    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int scancode, int action, int mods){
        auto r = static_cast<renderer*>(glfwGetWindowUserPointer(w));
        r->in.callback(w, key, scancode, action, mods);
    });

    if (headless) {
        glfwHideWindow(window);
    } else {
        glfwShowWindow(window);
    }
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

    std::vector<char const*> requiredLayers;
    if (enableValidationLayers) {
        requiredLayers.assign(validationLayers.begin(), validationLayers.end());
    }

    auto layerProperties = context.enumerateInstanceLayerProperties();
    auto unsupportedLayerIt = std::ranges::find_if(requiredLayers, [&layerProperties](auto const& requiredLayer) {
        return std::ranges::none_of(layerProperties, [requiredLayer](auto const& layerProperty) {
            return strcmp(layerProperty.layerName, requiredLayer) == 0;
        });
    });

    if (unsupportedLayerIt != requiredLayers.end()) {
        throw std::runtime_error("required layer not supported: " + std::string(*unsupportedLayerIt));
    }

    vk::InstanceCreateInfo createInfo {
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
        .ppEnabledLayerNames = requiredLayers.data(),
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

void renderer::vulkan_graphics_pipeline() {
    vk::raii::ShaderModule shaderModule = vutil::createShaderModule(ldevice, reinterpret_cast<const unsigned char*>(renderer::shader_bytes), renderer::shader_size);

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

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo {
        .vertexBindingDescriptionCount   = 0,
        .pVertexBindingDescriptions      = nullptr,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions    = nullptr
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
        .cullMode                = vk::CullModeFlagBits::eNone,
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
        .blendEnable         = vk::True,
        .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
        .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
        .colorBlendOp        = vk::BlendOp::eAdd,
        .srcAlphaBlendFactor = vk::BlendFactor::eOne,
        .dstAlphaBlendFactor = vk::BlendFactor::eZero,
        .alphaBlendOp        = vk::BlendOp::eAdd,
        .colorWriteMask      = vk::ColorComponentFlagBits::eR |
                               vk::ColorComponentFlagBits::eG |
                               vk::ColorComponentFlagBits::eB |
                               vk::ColorComponentFlagBits::eA
    };

    vk::PipelineColorBlendStateCreateInfo colorBlending {
        .logicOpEnable   = vk::False,
        .logicOp         = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments    = &colorBlendAttachment
    };

    vk::PushConstantRange pushRange {
        .stageFlags = vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex,
        .offset     = 0,
        .size       = 28  // float4 color + float minRadiusPx + float width + float height = 28 bytes
    };

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo {
        .setLayoutCount         = 1,
        .pSetLayouts            = &*descriptorSetLayout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges    = &pushRange
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

    for (size_t i = 0; i <MAX_FRAMES_IN_FLIGHT; i++) {
        presentCompleteSemaphores.emplace_back(ldevice, vk::SemaphoreCreateInfo());
        inFlightFences.emplace_back(ldevice, vk::FenceCreateInfo {
            .flags = vk::FenceCreateFlagBits::eSignaled
        });
    }

    for (size_t i = 0; i < swapchain.Images().size(); i++) {
        renderFinishedSemaphores.emplace_back(ldevice, vk::SemaphoreCreateInfo());
    }
}

void renderer::vulkan_init_descriptors() {
    std::array<vk::DescriptorPoolSize, 2> poolSize {{
        {.type = vk::DescriptorType::eStorageBuffer, .descriptorCount = 2 * MAX_FRAMES_IN_FLIGHT},
        { .type = vk::DescriptorType::eUniformBuffer, .descriptorCount = 1 * MAX_FRAMES_IN_FLIGHT},
    }};

    descriptorPool = vk::raii::DescriptorPool(ldevice, vk::DescriptorPoolCreateInfo {
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets       = MAX_FRAMES_IN_FLIGHT,
        .poolSizeCount = poolSize.size(),
        .pPoolSizes    = poolSize.data(),
    });

    std::array<vk::DescriptorSetLayoutBinding, 3> bindings {{
        { 0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr },
        { 1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr },
        { 2, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr },
    }};

    descriptorSetLayout = vk::raii::DescriptorSetLayout(ldevice, vk::DescriptorSetLayoutCreateInfo{
        .bindingCount = bindings.size(),
        .pBindings = bindings.data()
    });

    std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *descriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo {
        .descriptorPool = *descriptorPool,
        .descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
        .pSetLayouts = layouts.data()
    };

    descriptorSets = vk::raii::DescriptorSets{ldevice, allocInfo};
}

void renderer::vulkan_write_descriptors(size_t i, vk::Buffer state, vk::Buffer masses) {
    const vk::DescriptorBufferInfo stateInfo{state, 0, VK_WHOLE_SIZE};
    const vk::DescriptorBufferInfo massInfo{masses, 0, VK_WHOLE_SIZE};
    const vk::DescriptorBufferInfo uboInfo = uboBuffers[i].Info();

    const std::array<vk::WriteDescriptorSet, 3> writes{{
        {
            .dstSet = *descriptorSets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eStorageBuffer,
            .pBufferInfo = &stateInfo
        },
        {
            .dstSet = *descriptorSets[i],
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eStorageBuffer,
            .pBufferInfo = &massInfo
        },
        {
            .dstSet = *descriptorSets[i],
            .dstBinding = 2,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .pBufferInfo = &uboInfo
        }
    }};

    ldevice.Device().updateDescriptorSets(writes, {});
}

void renderer::vulkan_record_command_buffer(uint32_t imageIndex, simulation& sim, float ft) {
    auto& cmd = commandBuffers[frameIndex];
    cmd.begin({});

    vk::Buffer particleState{};
    vk::Buffer particleMasses{};

    if (sim.isGPU() && !Paused()) {
        sim.recordGPU(cmd, ft);
    }

    if (sim.isGPU()) {
        particleState = sim.gpuState();
        particleMasses = sim.gpuMasses();

        const vk::BufferMemoryBarrier2 computeToVertex{
            .srcStageMask = vk::PipelineStageFlagBits2::eComputeShader,
            .srcAccessMask = vk::AccessFlagBits2::eShaderStorageWrite,
            .dstStageMask = vk::PipelineStageFlagBits2::eVertexShader,
            .dstAccessMask = vk::AccessFlagBits2::eShaderStorageRead,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = particleState,
            .offset = 0,
            .size = VK_WHOLE_SIZE
        };
        const vk::DependencyInfo computeDependency{
            .bufferMemoryBarrierCount = 1,
            .pBufferMemoryBarriers = &computeToVertex
        };
        cmd.pipelineBarrier2(computeDependency);
    } else {
        const auto stateInfo = frames[frameIndex].stateInfo();
        const auto massInfo = frames[frameIndex].massInfo();
        particleState = stateInfo.buffer;
        particleMasses = massInfo.buffer;
    }

    // The frame fence was waited before recording, so this frame's set is no
    // longer in use and can safely be rebound to the selected state buffer.
    vulkan_write_descriptors(frameIndex, particleState, particleMasses);

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

    cmd.beginRendering(renderingInfo);
    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
    cmd.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        *pipelineLayout,
        0,
        { *descriptorSets[frameIndex] },
        {}
    );
    cmd.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapchain.Extent().width), static_cast<float>(swapchain.Extent().height), 0.0f, 1.0f));
    cmd.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapchain.Extent()));

    struct PushConstants { glm::vec4 color; float minRadiusPx; float width; float height; };
    PushConstants pc {
        {1.0f, 1.0f, 1.0f, 1.0f},
        MIN_RADIUS_PX,
        static_cast<float>(swapchain.Extent().width),
        static_cast<float>(swapchain.Extent().height)
    };

    cmd.pushConstants<PushConstants>(*pipelineLayout, vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex, 0, pc);
    cmd.draw(3, static_cast<uint32_t>(sim.points()), 0, 0);
    cmd.endRendering();

    vk::ImageLayout lastLayout = vk::ImageLayout::eColorAttachmentOptimal;
    vk::AccessFlags2 lastAccess = vk::AccessFlagBits2::eColorAttachmentWrite;
    vk::PipelineStageFlags2 lastStage = vk::PipelineStageFlagBits2::eColorAttachmentOutput;

    const bool grab = encoder.isOpen() && !recordLimitReached();
    if (grab && !capture.matches(swapchain.Extent())) {
        if (!recordSizeWarned) {
            fprintf(stderr, "window size changed while recording, skipping frames\n");
            recordSizeWarned = true;
        }
    } else if (grab) {
        transition_image_layout(
            imageIndex,
            lastLayout,
            vk::ImageLayout::eTransferSrcOptimal,
            lastAccess,
            vk::AccessFlagBits2::eTransferRead,
            lastStage,
            vk::PipelineStageFlagBits2::eCopy
        );

        capture.record(cmd, swapchain.Images()[imageIndex], frameIndex);

        lastLayout = vk::ImageLayout::eTransferSrcOptimal;
        lastAccess = {};
        lastStage = vk::PipelineStageFlagBits2::eCopy;
    }

    transition_image_layout(
        imageIndex,
        lastLayout,
        vk::ImageLayout::ePresentSrcKHR,
        lastAccess,
        {},
        lastStage,
        vk::PipelineStageFlagBits2::eBottomOfPipe
    );

    cmd.end();
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

void renderer::recreate_swapchain() {
    const auto previousFormat = swapchain.SurfaceFormat().format;
    if (!swapchain.recreate(pdevice, ldevice, surface, window)) {
        return;
    }

    if (swapchain.SurfaceFormat().format != previousFormat) {
        pipeline = nullptr;
        pipelineLayout = nullptr;
        vulkan_graphics_pipeline();
    }

    renderFinishedSemaphores.clear();
    for (size_t i = 0; i < swapchain.Images().size(); ++i) {
        renderFinishedSemaphores.emplace_back(ldevice, vk::SemaphoreCreateInfo{});
    }
}


std::chrono::nanoseconds renderer::render(simulation& sim, float dt, float ft) {
    const auto s = std::chrono::high_resolution_clock::now();
    glfwPollEvents();
    if (glfwWindowShouldClose(window)) {
        return std::chrono::high_resolution_clock::now() - s;
    }

    cam.update(window, dt);

    const auto fenceResult = ldevice.Device().waitForFences(*inFlightFences[frameIndex], vk::True, UINT64_MAX);
    if (fenceResult != vk::Result::eSuccess) {
        throw std::runtime_error("failed to wait for fence");
    }

    if (encoder.isOpen()) {
        if (const void* pixels = capture.take(frameIndex)) {
            encoder.push(pixels);
        }
    }

    auto [result, imageIndex] = swapchain.SwapChain().acquireNextImage(UINT64_MAX, *presentCompleteSemaphores[frameIndex], nullptr);
    if (result == vk::Result::eErrorOutOfDateKHR) {
        recreate_swapchain();
        return std::chrono::high_resolution_clock::now() - s;
    } else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("failed to acquire swap chain image");
    }

    if (recording() && !encoder.isOpen()) {
        start_recording();
    }

    if (!sim.isGPU()) {
        frames[frameIndex].update(sim.get_data());
    }

    const UBO ubo {
        .view = cam.viewMatrix(),
        .proj = cam.projMatrix(swapchain.Extent().width, swapchain.Extent().height)
    };
    uboBuffers[frameIndex].update(ubo);

    ldevice.Device().resetFences(*inFlightFences[frameIndex]);
    commandBuffers[frameIndex].reset();
    vulkan_record_command_buffer(imageIndex, sim, ft);

    const vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    const vk::SubmitInfo submitInfo {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*presentCompleteSemaphores[frameIndex],
        .pWaitDstStageMask = &waitDestinationStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &*commandBuffers[frameIndex],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &*renderFinishedSemaphores[imageIndex]
    };
    ldevice.Queue().submit(submitInfo, *inFlightFences[frameIndex]);

    const vk::PresentInfoKHR presentInfoKHR {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*renderFinishedSemaphores[imageIndex],
        .swapchainCount = 1,
        .pSwapchains = &*swapchain.SwapChain(),
        .pImageIndices = &imageIndex
    };

    result = ldevice.Queue().presentKHR(presentInfoKHR);
    if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR || framebufferResized) {
        framebufferResized = false;
        recreate_swapchain();
    }

    frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
    return std::chrono::high_resolution_clock::now() - s;
}

void renderer::waitIdle() {
    if (deviceInitialized) {
        ldevice.Device().waitIdle();
    }
}

void renderer::releaseSimulationReferences() {
    commandBuffers.clear();
    descriptorSets.clear();
    descriptorPool = nullptr;
}

void renderer::cleanup() {
    try {
        waitIdle();
    } catch (...) {}

    finishRecording();
    deviceInitialized = false;

    commandBuffers.clear();
    commandPool = nullptr;

    descriptorSets = nullptr;
    descriptorPool = nullptr;

    pipeline = nullptr;
    pipelineLayout = nullptr;
    descriptorSetLayout = nullptr;

    presentCompleteSemaphores.clear();
    renderFinishedSemaphores.clear();
    inFlightFences.clear();

    for (auto& f : frames) {
        f.cleanup();
    }

    for (auto& ubo : uboBuffers) {
        ubo.cleanup();
    }

    swapchain.cleanup();

    ldevice.cleanup();
    pdevice.cleanup();

    surface = nullptr;
    instance = nullptr;

    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    if (glfwInitialized) {
        glfwTerminate();
        glfwInitialized = false;
    }
}

void renderer::start_recording() {
    const auto format = swapchain.SurfaceFormat().format;
    const auto extent = swapchain.Extent();

    capture.init(ldevice, pdevice, extent, format, MAX_FRAMES_IN_FLIGHT);
    encoder.open(recordPath, extent.width, extent.height, recordFps, FrameCapture::ffmpegPixelFormat(format));
}

void renderer::finishRecording() {
    if (!encoder.isOpen()) return;

    try {
        waitIdle();

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (const void* pixels = capture.take((frameIndex + i) % MAX_FRAMES_IN_FLIGHT)) {
                encoder.push(pixels);
            }
        }
    } catch (const std::exception& e) {
        fprintf(stderr, "recording: %s\n", e.what());
    }

    encoder.close();
    capture.cleanup();
}
