#include "renderer.hpp"

std::chrono::nanoseconds renderer::render(const simulation& sim) {
    auto s = std::chrono::high_resolution_clock::now();

    // sim.renderable(ssbo);
    // float radius = 0.1f;

    // void* data;
    // vkMapMemory(device, stagingBufferMemory, 0, VK_WHOLE_SIZE, 0, &data);
    //     // Write radius first
    //     memcpy(data, &radius, sizeof(float));
    //     // Write array starting at offset 16 (to satisfy std140 alignment)
    //     memcpy((char*)data + 16, cpuCircles.data(), sizeof(CircleData) * sim.bodies());
    // vkUnmapMemory(device, stagingBufferMemory);

    // // 2. Copy Staging to Storage (Fast GPU-to-GPU transfer)
    // VkBufferCopy copyRegion{};
    // copyRegion.size = 16 + (sizeof(CircleData) * sim.bodies());
    // vkCmdCopyBuffer(commandBuffer, stagingBuffer, storageBuffer, 1, &copyRegion);

    // // 3. Add a barrier to ensure copy finishes before shader reads
    // VkBufferMemoryBarrier barrier{ VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER };
    // barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    // barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    // barrier.buffer = storageBuffer;
    // barrier.size = VK_WHOLE_SIZE;
    
    // vkCmdPipelineBarrier(commandBuffer, 
    //     VK_PIPELINE_STAGE_TRANSFER_BIT, 
    //     VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 
    //     0, 0, nullptr, 1, &barrier, 0, nullptr);

    // // 4. Draw n instances of the 6-vertex quad
    // vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, circlePipeline);
    
    // // (Bind descriptor sets for the SSBO here)
    // vkCmdDraw(commandBuffer, 6, sim.bodies(), 0, 0);

    return std::chrono::high_resolution_clock::now() - s;
}
