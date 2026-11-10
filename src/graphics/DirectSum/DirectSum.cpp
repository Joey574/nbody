#include "DirectSum.hpp"
#include "ParticleState.hpp"

#include <cstring>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace {
    alignas(uint32_t) constexpr unsigned char computeShaderBytes[] = {
        #embed "../../shaders/compiled/direct_sum.spv"
    };

    vk::raii::ShaderModule createShaderModule(
        const LogicalDevice& ld,
        const unsigned char* code,
        size_t size
    ) {
        if (size == 0 || size % sizeof(uint32_t) != 0) {
            throw std::runtime_error("invalid direct_sum.spv size");
        }

        vk::ShaderModuleCreateInfo info{
            .codeSize = size,
            .pCode = reinterpret_cast<const uint32_t*>(code)
        };
        return vk::raii::ShaderModule(ld.Device(), info);
    }

}

uint32_t DirectSum::findMemoryType(
    const PhysicalDevice& pd,
    uint32_t memoryTypeBits,
    vk::MemoryPropertyFlags properties
) {
    const auto memoryProperties = pd.Device().getMemoryProperties();
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
        const bool supported = (memoryTypeBits & (1u << i)) != 0;
        const bool hasProperties =
            (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties;
        if (supported && hasProperties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find a suitable Vulkan memory type");
}

void DirectSum::createBuffer(
    const LogicalDevice& ld,
    const PhysicalDevice& pd,
    vk::DeviceSize size,
    vk::BufferUsageFlags usage,
    vk::MemoryPropertyFlags properties,
    vk::raii::Buffer& buffer,
    vk::raii::DeviceMemory& memory
) {
    if (size == 0) {
        throw std::invalid_argument("cannot create a zero-sized GPU buffer");
    }

    const vk::BufferCreateInfo bufferInfo{
        .size = size,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive
    };
    buffer = vk::raii::Buffer(ld.Device(), bufferInfo);

    const auto requirements = buffer.getMemoryRequirements();
    const vk::MemoryAllocateInfo allocation{
        .allocationSize = requirements.size,
        .memoryTypeIndex = findMemoryType(pd, requirements.memoryTypeBits, properties)
    };
    memory = vk::raii::DeviceMemory(ld.Device(), allocation);
    buffer.bindMemory(*memory, 0);
}

void DirectSum::init(
    const data& initialData,
    const LogicalDevice& ld,
    const PhysicalDevice& pd
) {
    cleanup();

    if (initialData.bodies() == 0) {
        throw std::invalid_argument("GPU simulation requires at least one particle");
    }
    if (initialData.bodies() > std::numeric_limits<uint32_t>::max()) {
        throw std::length_error("particle count exceeds the GPU shader's uint range");
    }

    count = static_cast<uint32_t>(initialData.bodies());
    currentIndex = 0;

    const vk::DeviceSize stateBytes =
        static_cast<vk::DeviceSize>(count) * sizeof(ParticleState);
    const vk::DeviceSize massBytes =
        static_cast<vk::DeviceSize>(count) * sizeof(float);

    const auto limits = pd.Device().getProperties().limits;
    if (stateBytes > limits.maxStorageBufferRange ||
        massBytes > limits.maxStorageBufferRange) {
        throw std::length_error("particle buffers exceed maxStorageBufferRange");
    }
    if (BLOCK_SIZE > limits.maxComputeWorkGroupInvocations ||
        BLOCK_SIZE > limits.maxComputeWorkGroupSize[0]) {
        throw std::runtime_error("selected Vulkan device cannot run the configured compute workgroup size");
    }
    constexpr uint32_t sharedMemoryBytes = BLOCK_SIZE * (2 * sizeof(float) + sizeof(float));
    if (sharedMemoryBytes > limits.maxComputeSharedMemorySize) {
        throw std::runtime_error("selected Vulkan device has insufficient compute shared memory");
    }
    if (sizeof(Params) > limits.maxPushConstantsSize) {
        throw std::runtime_error("selected Vulkan device has insufficient push-constant storage");
    }
    const uint64_t groupCount =
        (static_cast<uint64_t>(count) + BLOCK_SIZE - 1) / BLOCK_SIZE;
    if (groupCount > limits.maxComputeWorkGroupCount[0]) {
        throw std::length_error("particle count exceeds maxComputeWorkGroupCount[0]");
    }

    createBuffer(
        ld, pd, stateBytes,
        vk::BufferUsageFlagBits::eStorageBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        stateBuffers[0], stateMemories[0]
    );
    createBuffer(
        ld, pd, stateBytes,
        vk::BufferUsageFlagBits::eStorageBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        stateBuffers[1], stateMemories[1]
    );
    createBuffer(
        ld, pd, massBytes,
        vk::BufferUsageFlagBits::eStorageBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        massesBuffer, massesMemory
    );

    vk::raii::DeviceMemory stateStagingMemory = nullptr;
    vk::raii::Buffer stateStaging = nullptr;
    vk::raii::DeviceMemory massStagingMemory = nullptr;
    vk::raii::Buffer massStaging = nullptr;

    auto createStaging = [&](vk::DeviceSize size,
                             vk::raii::Buffer& buffer,
                             vk::raii::DeviceMemory& memory) {
        const vk::BufferCreateInfo bufferInfo{
            .size = size,
            .usage = vk::BufferUsageFlagBits::eTransferSrc,
            .sharingMode = vk::SharingMode::eExclusive
        };
        buffer = vk::raii::Buffer(ld.Device(), bufferInfo);
        const auto requirements = buffer.getMemoryRequirements();
        const vk::MemoryAllocateInfo allocation{
            .allocationSize = requirements.size,
            .memoryTypeIndex = findMemoryType(
                pd,
                requirements.memoryTypeBits,
                vk::MemoryPropertyFlagBits::eHostVisible |
                vk::MemoryPropertyFlagBits::eHostCoherent
            )
        };
        memory = vk::raii::DeviceMemory(ld.Device(), allocation);
        buffer.bindMemory(*memory, 0);
    };

    createStaging(stateBytes, stateStaging, stateStagingMemory);
    createStaging(massBytes, massStaging, massStagingMemory);

    auto* mappedState = static_cast<ParticleState*>(
        stateStagingMemory.mapMemory(0, stateBytes)
    );
    for (uint32_t i = 0; i < count; ++i) {
        mappedState[i] = {
            initialData.posx()[i],
            initialData.posy()[i],
            initialData.velx()[i],
            initialData.vely()[i]
        };
    }
    stateStagingMemory.unmapMemory();

    void* mappedMasses = massStagingMemory.mapMemory(0, massBytes);
    std::memcpy(mappedMasses, initialData.mass(), static_cast<size_t>(massBytes));
    massStagingMemory.unmapMemory();

    vk::CommandPoolCreateInfo poolInfo{
        .flags = vk::CommandPoolCreateFlagBits::eTransient,
        .queueFamilyIndex = ld.QueueIdx()
    };
    vk::raii::CommandPool uploadPool(ld.Device(), poolInfo);
    vk::CommandBufferAllocateInfo allocateInfo{
        .commandPool = *uploadPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };
    auto uploadCommands = vk::raii::CommandBuffers(ld.Device(), allocateInfo);
    auto& uploadCmd = uploadCommands[0];
    uploadCmd.begin(vk::CommandBufferBeginInfo{
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    });
    uploadCmd.copyBuffer(*stateStaging, *stateBuffers[0], vk::BufferCopy{0, 0, stateBytes});
    uploadCmd.copyBuffer(*massStaging, *massesBuffer, vk::BufferCopy{0, 0, massBytes});

    std::array<vk::BufferMemoryBarrier2, 2> uploadBarriers{{
        {
            .srcStageMask = vk::PipelineStageFlagBits2::eTransfer,
            .srcAccessMask = vk::AccessFlagBits2::eTransferWrite,
            .dstStageMask = vk::PipelineStageFlagBits2::eComputeShader |
                            vk::PipelineStageFlagBits2::eVertexShader,
            .dstAccessMask = vk::AccessFlagBits2::eShaderStorageRead,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = *stateBuffers[0],
            .offset = 0,
            .size = stateBytes
        },
        {
            .srcStageMask = vk::PipelineStageFlagBits2::eTransfer,
            .srcAccessMask = vk::AccessFlagBits2::eTransferWrite,
            .dstStageMask = vk::PipelineStageFlagBits2::eComputeShader |
                            vk::PipelineStageFlagBits2::eVertexShader,
            .dstAccessMask = vk::AccessFlagBits2::eShaderStorageRead,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = *massesBuffer,
            .offset = 0,
            .size = massBytes
        }
    }};
    const vk::DependencyInfo uploadDependency{
        .bufferMemoryBarrierCount = static_cast<uint32_t>(uploadBarriers.size()),
        .pBufferMemoryBarriers = uploadBarriers.data()
    };
    uploadCmd.pipelineBarrier2(uploadDependency);
    uploadCmd.end();

    const vk::SubmitInfo uploadSubmit{
        .commandBufferCount = 1,
        .pCommandBuffers = &*uploadCmd
    };
    ld.Queue().submit(uploadSubmit, vk::Fence{});
    ld.Queue().waitIdle();

    stateStaging = nullptr;
    massStaging = nullptr;
    stateStagingMemory = nullptr;
    massStagingMemory = nullptr;

    const std::array<vk::DescriptorSetLayoutBinding, 3> bindings{{
        {0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute, nullptr},
        {1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute, nullptr},
        {2, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute, nullptr}
    }};
    descriptorLayout = vk::raii::DescriptorSetLayout(
        ld.Device(),
        vk::DescriptorSetLayoutCreateInfo{
            .bindingCount = static_cast<uint32_t>(bindings.size()),
            .pBindings = bindings.data()
        }
    );

    const vk::DescriptorPoolSize poolSize{
        .type = vk::DescriptorType::eStorageBuffer,
        .descriptorCount = 6
    };
    descriptorPool = vk::raii::DescriptorPool(
        ld.Device(),
        vk::DescriptorPoolCreateInfo{
            .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
            .maxSets = 2,
            .poolSizeCount = 1,
            .pPoolSizes = &poolSize
        }
    );

    const std::array<vk::DescriptorSetLayout, 2> layouts{
        *descriptorLayout, *descriptorLayout
    };
    descriptorSets = ld.Device().allocateDescriptorSets(vk::DescriptorSetAllocateInfo{
        .descriptorPool = *descriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data()
    });

    const std::array<vk::DescriptorBufferInfo, 6> bufferInfos{{
        {*stateBuffers[0], 0, stateBytes},
        {*stateBuffers[1], 0, stateBytes},
        {*massesBuffer, 0, massBytes},
        {*stateBuffers[1], 0, stateBytes},
        {*stateBuffers[0], 0, stateBytes},
        {*massesBuffer, 0, massBytes}
    }};
    std::array<vk::WriteDescriptorSet, 6> writes{};
    for (uint32_t set = 0; set < 2; ++set) {
        for (uint32_t binding = 0; binding < 3; ++binding) {
            const uint32_t infoIndex = set * 3 + binding;
            writes[infoIndex] = vk::WriteDescriptorSet{
                .dstSet = *descriptorSets[set],
                .dstBinding = binding,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eStorageBuffer,
                .pBufferInfo = &bufferInfos[infoIndex]
            };
        }
    }
    ld.Device().updateDescriptorSets(writes, {});

    const vk::PushConstantRange pushRange{
        .stageFlags = vk::ShaderStageFlagBits::eCompute,
        .offset = 0,
        .size = sizeof(Params)
    };
    pipelineLayout = vk::raii::PipelineLayout(
        ld.Device(),
        vk::PipelineLayoutCreateInfo{
            .setLayoutCount = 1,
            .pSetLayouts = &*descriptorLayout,
            .pushConstantRangeCount = 1,
            .pPushConstantRanges = &pushRange
        }
    );

    auto shaderModule = createShaderModule(ld, computeShaderBytes, sizeof(computeShaderBytes));
    const vk::ComputePipelineCreateInfo pipelineInfo{
        .stage = vk::PipelineShaderStageCreateInfo{
            .stage = vk::ShaderStageFlagBits::eCompute,
            .module = *shaderModule,
            .pName = "computeMain"
        },
        .layout = *pipelineLayout
    };
    pipeline = vk::raii::Pipeline(ld.Device(), nullptr, pipelineInfo);
}

void DirectSum::record(const vk::raii::CommandBuffer& cmd, float dt) {
    if (count == 0) {
        throw std::logic_error("GPU simulation has not been initialized");
    }
    if (!std::isfinite(dt) || dt <= 0.0f) {
        throw std::invalid_argument("GPU simulation timestep must be positive and finite");
    }

    const vk::MemoryBarrier2 reuseBarrier{
        .srcStageMask = vk::PipelineStageFlagBits2::eComputeShader |
                        vk::PipelineStageFlagBits2::eVertexShader,
        .srcAccessMask = vk::AccessFlagBits2::eShaderStorageWrite |
                         vk::AccessFlagBits2::eShaderStorageRead,
        .dstStageMask = vk::PipelineStageFlagBits2::eComputeShader,
        .dstAccessMask = vk::AccessFlagBits2::eShaderStorageRead |
                         vk::AccessFlagBits2::eShaderStorageWrite
    };
    const vk::DependencyInfo reuseDependency{
        .memoryBarrierCount = 1,
        .pMemoryBarriers = &reuseBarrier
    };
    cmd.pipelineBarrier2(reuseDependency);

    const Params params{
        .samples = count,
        .dt = dt,
    };

    cmd.bindPipeline(vk::PipelineBindPoint::eCompute, *pipeline);
    cmd.bindDescriptorSets(
        vk::PipelineBindPoint::eCompute,
        *pipelineLayout,
        0,
        {*descriptorSets[currentIndex]},
        {}
    );
    cmd.pushConstants<Params>(
        *pipelineLayout,
        vk::ShaderStageFlagBits::eCompute,
        0,
        params
    );

    const uint32_t groups = static_cast<uint32_t>(
        (static_cast<uint64_t>(count) + BLOCK_SIZE - 1) / BLOCK_SIZE
    );
    cmd.dispatch(groups, 1, 1);

    currentIndex ^= 1;
}

vk::Buffer DirectSum::currentState() const {
    if (count == 0) {
        throw std::logic_error("GPU simulation state requested before initialization");
    }
    return *stateBuffers[currentIndex];
}

vk::Buffer DirectSum::massBuffer() const {
    if (count == 0) {
        throw std::logic_error("GPU simulation masses requested before initialization");
    }
    return *massesBuffer;
}

void DirectSum::cleanup() {
    pipeline = nullptr;
    pipelineLayout = nullptr;
    descriptorSets.clear();
    descriptorPool = nullptr;
    descriptorLayout = nullptr;

    massesBuffer = nullptr;
    massesMemory = nullptr;
    for (size_t i = 0; i < stateBuffers.size(); ++i) {
        stateBuffers[i] = nullptr;
        stateMemories[i] = nullptr;
    }

    count = 0;
    currentIndex = 0;
}
