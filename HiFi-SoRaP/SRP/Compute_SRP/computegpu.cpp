#include "computegpu.h"
#include "qfileinfo.h"
#include <optional>
#include <array>

struct UniformBufferObject {
    vector3 lightDirection;
    vector3 worldCamPos;
    Eigen::Matrix4f model;
    int32_t debugMode;
    vector3 diffuse;
};


ComputeGPU::ComputeGPU(VkInstance instance) {
    this->instance = instance;
}



// Support structs and typedefs

void ComputeGPU::init() {

    pickPhysicalDevice();
    createLogicalDevice();
    createComputeDescriptorSetLayout();
    createComputePipeline();
    createCommandPool();
    createShaderStorageBuffers();
    createUniformBuffers();
    createDescriptorPool();
    createComputeDescriptorSets();
    createComputeCommandBuffers();
    createSyncObjects();
}

void ComputeGPU::process() {
    updateUniforms();
    recordComputeCommandBuffer(computeCommandBuffer);

    auto start = std::chrono::high_resolution_clock::now();
    dispatchCompute();
    waitForComputeWork();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "Compute work took " << duration.count() << " microseconds" << std::endl;

    writeBackCPU();
}


/**
 *  Enumerate physical devices and select "best" one
 */
void ComputeGPU::pickPhysicalDevice() {

    VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
    PhysicalDeviceProps bestDeviceProps;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if(deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support");
    }
    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

    std::vector<PhysicalDeviceProps> physicalDeviceProps(physicalDevices.size());
    for (int i = 0; i < physicalDevices.size(); ++i)
    {
        vkGetPhysicalDeviceProperties(physicalDevices[i], &physicalDeviceProps[i].m_Properties);
        vkGetPhysicalDeviceMemoryProperties(physicalDevices[i], &physicalDeviceProps[i].m_MemoryProperties);


        // queue family properties
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyCount, nullptr);
        physicalDeviceProps[i].m_QueueFamilyProperties.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyCount, physicalDeviceProps[i].m_QueueFamilyProperties.data());

        // device layer properties
        uint32_t layerPropCount = 0;
        vkEnumerateDeviceLayerProperties(physicalDevices[i], &layerPropCount, nullptr);
        physicalDeviceProps[i].m_LayerProperties.resize(layerPropCount);
        vkEnumerateDeviceLayerProperties(physicalDevices[i], &layerPropCount, physicalDeviceProps[i].m_LayerProperties.data());

        // device extension properties
        uint32_t extensionPropCount = 0;
        vkEnumerateDeviceExtensionProperties(physicalDevices[i], nullptr,  &extensionPropCount, nullptr);
        physicalDeviceProps[i].m_ExtensionProperties.resize(extensionPropCount);
        vkEnumerateDeviceExtensionProperties(physicalDevices[i], nullptr, &extensionPropCount, physicalDeviceProps[i].m_ExtensionProperties.data());
    }

    uint64_t bestDeviceIndex = 0;
    for (int i = 1; i < physicalDevices.size(); ++i)
    {
        const bool isDiscrete = physicalDeviceProps[bestDeviceIndex].m_Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        const bool otherIsDiscrete = physicalDeviceProps[i].m_Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        if (isDiscrete && !otherIsDiscrete)
            continue;
        else if ((!isDiscrete && otherIsDiscrete)
                 || (physicalDeviceProps[bestDeviceIndex].m_Properties.limits.maxFramebufferWidth < physicalDeviceProps[i].m_Properties.limits.maxFramebufferWidth))
            bestDeviceIndex = i;
    }

    this->physicalDevice = physicalDevices[bestDeviceIndex];
    this->physicalDeviceProps = physicalDeviceProps[bestDeviceIndex];

}

/**
 * @brief Find a queue family with compute support within physicalDevice
 * @return
 */
uint32_t ComputeGPU::findQueueFamilies() {
    uint32_t computeQueue = 0;
    while(computeQueue < physicalDeviceProps.m_QueueFamilyProperties.size()) {
        if(physicalDeviceProps.m_QueueFamilyProperties[computeQueue].queueFlags &
            VK_QUEUE_COMPUTE_BIT) {
            break;
        }
        computeQueue++;

    }

    return computeQueue;
}


/**
 * @brief Create a VkDevice with the selected physical device
 * and retrieve compute queue.
 */
void ComputeGPU::createLogicalDevice() {
    uint32_t queueFamilyIndex = findQueueFamilies();

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;


    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);


    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    //vkGetDeviceQueue(device, indices.graphicsAndComputeFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, queueFamilyIndex, 0, &computeQueue);
    //vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

/**
 * @brief Crate descriptor set layouts describing compute shader resources
 */
void ComputeGPU::createComputeDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding layoutBindings[2];

    layoutBindings[0].binding = 0;
    layoutBindings[0].descriptorCount = 1;
    layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBindings[0].pImmutableSamplers = nullptr;
    layoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    layoutBindings[1].binding = 1;
    layoutBindings[1].descriptorCount = 1;
    layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBindings[1].pImmutableSamplers = nullptr;
    layoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 2;
    layoutInfo.pBindings = layoutBindings;

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &computeDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute descriptor set layout!");
    }

}


VkShaderModule ComputeGPU::createShaderModule(const QString &name)
{
    QFile file(name);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Failed to read shader %s", qPrintable(name));
        return VK_NULL_HANDLE;
    }
    QByteArray blob = file.readAll();
    file.close();

    VkShaderModuleCreateInfo shaderInfo;
    memset(&shaderInfo, 0, sizeof(shaderInfo));
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = blob.size();
    shaderInfo.pCode = reinterpret_cast<const uint32_t *>(blob.constData());
    VkShaderModule shaderModule;
    VkResult err = vkCreateShaderModule(device, &shaderInfo, nullptr, &shaderModule);
    if (err != VK_SUCCESS) {
        qWarning("Failed to create shader module: %d", err);
        return VK_NULL_HANDLE;
    }

    return shaderModule;
}



void ComputeGPU::createComputePipeline() {

    VkShaderModule computeShaderModule = createShaderModule(computeShaderFilename);

    VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
    computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageInfo.module = computeShaderModule;
    computeShaderStageInfo.pName = "main";

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &computeDescriptorSetLayout;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &computePipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline layout!");
    }

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = computePipelineLayout;
    pipelineInfo.stage = computeShaderStageInfo;

    if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline!");
    }

    vkDestroyShaderModule(device, computeShaderModule, nullptr);
}

/**
 * @brief Create a command pool to get compute GPU commands.
 */
void ComputeGPU::createCommandPool() {
    uint32_t queueFamilyIndex = findQueueFamilies();

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndex;

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }
}



uint32_t ComputeGPU::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}



/**
 * @brief Utility function to create and allocate a GPU buffer.
 * @param size
 * @param usage
 * @param properties
 * @param buffer
 * @param bufferMemory
 */
void ComputeGPU::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void ComputeGPU::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    // create a command buffer to record the device copy command
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(computeQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(computeQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}


#define DATA_COUNT 1024 * 1024 * 25

void ComputeGPU::createShaderStorageBuffers() {

    std::vector<float> sampleData(DATA_COUNT, 0.0f);

    VkDeviceSize bufferSize = DATA_COUNT * sizeof(float);

    // Create a staging buffer used to upload data to the gpu
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, sampleData.data(), (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);


    // Copy data to all storage buffers
    createBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT
                                 | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, shaderStorageBuffer, shaderStorageBufferMemory);
    copyBuffer(stagingBuffer, shaderStorageBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void ComputeGPU::createUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffer, uniformBufferMemory);

    vkMapMemory(device, uniformBufferMemory, 0, bufferSize, 0, &uniformBufferMapped);

}

void ComputeGPU::createDescriptorPool() {
    VkDescriptorPoolSize poolSizes[2];
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(1);

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(1);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = static_cast<uint32_t>(1);

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void ComputeGPU::createComputeDescriptorSets() {
    VkDescriptorSetLayout layout = computeDescriptorSetLayout;

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
    allocInfo.pSetLayouts = &layout;

    if (vkAllocateDescriptorSets(device, &allocInfo, &computeDescriptorSets) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }


    VkDescriptorBufferInfo uniformBufferInfo{};
    uniformBufferInfo.buffer = uniformBuffer;
    uniformBufferInfo.offset = 0;
    uniformBufferInfo.range = sizeof(UniformBufferObject);

    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = computeDescriptorSets;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &uniformBufferInfo;

    VkDescriptorBufferInfo storageBufferInfo{};
    storageBufferInfo.buffer = shaderStorageBuffer;
    storageBufferInfo.offset = 0;
    storageBufferInfo.range = sizeof(float) * DATA_COUNT;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = computeDescriptorSets;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = &storageBufferInfo;

    vkUpdateDescriptorSets(device, 2, descriptorWrites.data(), 0, nullptr);

}

void ComputeGPU::createComputeCommandBuffers() {

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(device, &allocInfo, &computeCommandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate compute command buffers!");
    }
}

void ComputeGPU::createSyncObjects() {

    // create a fence with "unsignaled" initial state
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = 0;

    if (vkCreateFence(device, &fenceInfo, nullptr, &computeFence) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute fence");
    }
}


namespace {

const float kFieldOfView = 60;
const float kZNear = 0.0001;
const float kZFar = 800;

}
void ComputeGPU::updateUniforms() {

    camera.setViewport(0, 0, 520, 520);
    camera.setProjection(kFieldOfView, kZNear, kZFar);

    Eigen::Matrix4f view = camera.setView();

    this->light = new Light();

    UniformBufferObject ubo{};
    ubo.lightDirection = light->getLightDir();
    ubo.model = camera.setModel();
    ubo.debugMode = 2;
    ubo.diffuse = vector3(1.0f, 0.0f, 0.0f);

    // get camera translation via its matrix
    Eigen::Vector4f camPosM = view.inverse().col(3);
    vector3 camPos = vector3(camPosM[0], camPosM[1], camPosM[2]);
    ubo.worldCamPos = camPos;

    memcpy(uniformBufferMapped, &ubo, sizeof(ubo));
}


void ComputeGPU::recordComputeCommandBuffer(VkCommandBuffer commandBuffer) {
    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording compute command buffer!");
    }

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout, 0, 1, &computeDescriptorSets, 0, nullptr);

    vkCmdDispatch(commandBuffer, DATA_COUNT / 256, 1, 1);


    VkMemoryBarrier memoryBarrier{};
    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    memoryBarrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    memoryBarrier.pNext = nullptr;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         VK_PIPELINE_STAGE_HOST_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record compute command buffer!");
    }
}


void ComputeGPU::dispatchCompute() {

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    vkResetCommandBuffer(computeCommandBuffer, 0);
    recordComputeCommandBuffer(computeCommandBuffer);

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &computeCommandBuffer;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;

    if(vkQueueSubmit(computeQueue, 1, &submitInfo, computeFence) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit compute command buffer");
    }

}

void ComputeGPU::waitForComputeWork() {
    // wait for compute work to finish
    vkWaitForFences(device, 1, &computeFence, VK_TRUE, UINT64_MAX);
}

void ComputeGPU::writeBackCPU() {
    // Create a staging buffer used to retrieve data to the gpu
    // is this the best way to do it??
    VkDeviceSize bufferSize = DATA_COUNT * sizeof(float);
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    copyBuffer(shaderStorageBuffer, stagingBuffer, bufferSize);

    std::vector<float> data(DATA_COUNT);

    float* bufferData;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, (void **) &bufferData);
    memcpy(data.data(), bufferData, (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    // print data
    std::cout << data[0] << " ";
    std::cout << data[1] << " ";
    std::cout << data[2] << " ";
    std::cout << data[3] << " ";
    std::cout << data[4] << " ";
    std::cout << data[5] << " ";
    std::cout << data[6] << " ";
    std::cout << data[7] << " ";
    std::cout << data[8] << " ";
    std::cout << data[9] << " ";
    for(uint32_t i = 0; i < DATA_COUNT; i = i + 1024 * 100) {
        std::cout << data[i] << " ";
    }
    std::cout << std::flush;

}

void ComputeGPU::cleanup() {

    vkDestroyPipeline(device, computePipeline, nullptr);
    vkDestroyPipelineLayout(device, computePipelineLayout, nullptr);



    vkDestroyBuffer(device, uniformBuffer, nullptr);
    vkFreeMemory(device, uniformBufferMemory, nullptr);

    vkDestroyDescriptorPool(device, descriptorPool, nullptr);

    vkDestroyDescriptorSetLayout(device, computeDescriptorSetLayout, nullptr);

    vkDestroyBuffer(device, shaderStorageBuffer, nullptr);
    vkFreeMemory(device, shaderStorageBufferMemory, nullptr);


    vkDestroyFence(device, computeFence, nullptr);

    vkDestroyCommandPool(device, commandPool, nullptr);

    vkDestroyDevice(device, nullptr);

}



