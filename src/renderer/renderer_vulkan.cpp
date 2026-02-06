#include "renderer.hpp"

int renderer::init_vulkan(GLFWwindow* window, size_t n) {
    //if (create_buffers(n)) { return 1; }
    if (create_instance()) { return 1; }
    if (create_surface(window)) { return 1; }
    //if (create_device()) { return 1; }
    //if (create_pipeline()) { return 1; }
    //if (set_descriptors(n)) { return 1; }
    return 0;
}

int renderer::create_buffers(size_t n) {
    return 0;
}

int renderer::create_instance() {
    
    // vulkan initialization
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Window";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        return 1;
    }

    return 0;
}

int renderer::create_surface(GLFWwindow* window) {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        return 1;
    }

    return 0;
}

int renderer::create_device() {
    return 0;
}

int renderer::create_pipeline() {
    return 0;
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout);

    //vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &circlePipeline);

    return 0;
}

int renderer::set_descriptors(size_t n) {
    return 0;
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = 0;
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &layoutBinding;
    vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout);

    VkDescriptorSetAllocateInfo allocInfo {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = storageBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = 16 + (sizeof(CircleData) * n);

    VkWriteDescriptorSet descriptorWrite {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrite.pBufferInfo = &bufferInfo;
    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);

    return 0;
}

void renderer::cleanup() {
    if (device) { vkDeviceWaitIdle(device); }

    if (circlePipeline) { vkDestroyPipeline(device, circlePipeline, nullptr); }
    if (pipelineLayout) { vkDestroyPipelineLayout(device, pipelineLayout, nullptr); }

    if (stagingBuffer) { vkDestroyBuffer(device, stagingBuffer, nullptr); }
    if (stagingBufferMemory) { vkFreeMemory(device, stagingBufferMemory, nullptr); }
    if (storageBuffer) { vkDestroyBuffer(device, storageBuffer, nullptr); }
    if (storageBufferMemory) { vkFreeMemory(device, storageBufferMemory, nullptr); }

    if (descriptorPool) {vkDestroyDescriptorPool(device, descriptorPool, nullptr); }
    if (descriptorSetLayout) {vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr); }

    if (commandPool) { vkDestroyCommandPool(device, commandPool, nullptr); }
    if (device) { vkDestroyDevice(device, nullptr); }
    if (surface ) { vkDestroySurfaceKHR(instance, surface, nullptr); }
    if (instance) { vkDestroyInstance(instance, nullptr); }
}