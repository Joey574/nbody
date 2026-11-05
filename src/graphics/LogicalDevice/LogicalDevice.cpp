#include "LogicalDevice.hpp"

void LogicalDevice::init(const PhysicalDevice& pdevice, const vk::raii::SurfaceKHR& surface) {
    const auto& pd = pdevice.Device();

    auto queueFamilyProperties = pd.getQueueFamilyProperties();
    for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++) {
        if (queueFamilyProperties[qfpIndex].queueCount == 0) continue;

        const auto flags = queueFamilyProperties[qfpIndex].queueFlags;
        const bool graphics = bool(flags & vk::QueueFlagBits::eGraphics);
        const bool compute = bool(flags & vk::QueueFlagBits::eCompute);
        const bool present = pd.getSurfaceSupportKHR(qfpIndex, surface);

        if (graphics && compute && present) {
            queueIdx = qfpIndex;
            break;
        }
    }

    if (queueIdx == ~0) {
        throw std::runtime_error("could not find a queue for graphics, present, and compute");
    }

    vk::StructureChain<
        vk::PhysicalDeviceFeatures2,
        vk::PhysicalDeviceVulkan11Features,
        vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
        featureChain = {
            {},
            {.shaderDrawParameters = true},
            {.synchronization2 = true, .dynamicRendering = true},
            {.extendedDynamicState = true}
        };

    float queuePriority = 0.5f;
    vk::DeviceQueueCreateInfo queueCreateInfo {
        .queueFamilyIndex = queueIdx,
        .queueCount       = 1,
        .pQueuePriorities = &queuePriority
    };

    vk::DeviceCreateInfo deviceCreateInfo {
        .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCreateInfo,
        .enabledExtensionCount = static_cast<uint32_t>(std::size(deviceExtensions)),
        .ppEnabledExtensionNames = deviceExtensions
    };

    device = vk::raii::Device(pd, deviceCreateInfo);
    queue = vk::raii::Queue(device, queueIdx, 0);
}

void LogicalDevice::cleanup() {
    queue = nullptr;
    device = nullptr;
}
