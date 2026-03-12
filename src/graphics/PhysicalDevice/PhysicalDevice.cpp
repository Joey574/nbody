#include "PhysicalDevice.hpp"
#include <map>

void PhysicalDevice::init(const vk::raii::Instance& instance) {
    auto devices = instance.enumeratePhysicalDevices();
    if (devices.empty()) {
        throw std::runtime_error("no devices");
    }

    std::multimap<int, vk::raii::PhysicalDevice> candidates;

    for (const auto& d : devices) {
        auto deviceProperties = d.getProperties();
        auto deviceFeatures = d.getFeatures();
        uint32_t score = 0;

        // can't do geometry, skip
        if (!deviceFeatures.geometryShader) {
            continue;
        }

        // favor dedicated gpus
        if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
            score += 1000;
        }

        // favor integrated gpus, less than dedicated
        if (deviceProperties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu) {
            score += 100;
        }

        score += deviceProperties.limits.maxImageDimension2D;
        candidates.insert(std::make_pair(score, d));
    }

    if (candidates.rbegin()->first > 0) {
        device = candidates.rbegin()->second;
    } else {
        throw std::runtime_error("no suitable device");
    }
}
