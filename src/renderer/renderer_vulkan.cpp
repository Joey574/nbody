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

    return 0;
}

/// @brief Initializes the main application window
/// @return 0 if successful
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
    vk::KHRSwapchainExtensionName};

    vk::DeviceCreateInfo deviceCreateInfo{
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

void renderer::cleanup() {
    if (window) { glfwDestroyWindow(window); }
    glfwTerminate();
}
