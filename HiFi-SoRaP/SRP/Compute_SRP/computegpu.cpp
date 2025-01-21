#include "computegpu.h"
#include "qfileinfo.h"
#include <optional>
#include <array>
#include <cmath>


const char* vkResultToString(VkResult result) {
    switch (result) {
    case VK_SUCCESS: return "VK_SUCCESS";
    case VK_NOT_READY: return "VK_NOT_READY";
    case VK_TIMEOUT: return "VK_TIMEOUT";
    case VK_EVENT_SET: return "VK_EVENT_SET";
    case VK_EVENT_RESET: return "VK_EVENT_RESET";
    case VK_INCOMPLETE: return "VK_INCOMPLETE";
    case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
    case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
    case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
    case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
    case VK_ERROR_OUT_OF_POOL_MEMORY: return "VK_ERROR_OUT_OF_POOL_MEMORY";
    case VK_ERROR_INVALID_EXTERNAL_HANDLE: return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
    default: return "UNKNOWN_ERROR";
    }
}




ComputeGPU::ComputeGPU(VkInstance instance) {
    this->instance = instance;
    // default values
    this->width = 512;
    this->height = 512;
}


std::vector<cTriangle> ComputeGPU::prepareTriangles() {
    if (this->satellite == nullptr ) {
    }

    std::vector<cTriangle> ctriangles;
    TriangleMesh* pMesh = satellite->getMesh();
    numTriangles = pMesh->faces.size();

    for(uint32_t i = 0; i < pMesh->faces.size(); i++ ) {

        int i0 = pMesh->faces[i].v1;
        int i1 = pMesh->faces[i].v2;
        int i2 = pMesh->faces[i].v3;


        cTriangle t {
            pMesh->vertices[i0],
            pMesh->vertices[i1],
            pMesh->vertices[i2],
            (int32_t)pMesh->faces[i].rf
        };
        ctriangles.push_back(t);

        //std::cout << t.v1 << " " << t.v2 << " " << t.v3 << " " << t.materialIndex << " ";
    }

    return ctriangles;
}

std::vector<cMaterial> ComputeGPU::prepareMaterials() {
    if (this->satellite == nullptr ) {
    }

    std::vector<cMaterial> cmaterials;
    Object* object = this->satellite;
    numMaterials = object->getNumMaterials();

    for(uint32_t i = 0; i < object->getNumMaterials(); i++ ) {

        Material m1 = object->getMaterial(i);

        cMaterial mcp {
            m1.ps,
            m1.pd,
            m1.refIdx,
            (int32_t) m1.r,
        };
        cmaterials.push_back(mcp);
    }

    return cmaterials;
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


void ComputeGPU::computeStepSRP(const vector3& XS, vector3 &force, const vector3& V1, const vector3& V2) {
    //camera.setViewport(0, 0, 512, 512);
    //camera.setProjection(kFieldOfView, kZNear, kZFar);

    Eigen::Matrix4f view = camera.setView();

    int time_seed = getTimeSeed();

    TriangleMesh *mesh = this->satellite->getMesh();
    Eigen::Vector3f diff = mesh->max_-mesh->min_;
    float diagonalDiff = diff.norm();
    float errorMargin = 0.1f;
    this->distance = diagonalDiff+errorMargin;

    //float xAxis = distance/2.0f;
    //float yAxis = distance/2.0f;

    UniformBufferObject ubo{};
    ubo.lightDirection = XS;
    ubo.V1 = V1;
    ubo.V2 = V2;
    ubo.model = camera.setModel();
    ubo.worldCamPos = vector3(0.0f, 0.0f, 0.0f);
    ubo.debugMode = 2;
    ubo.diffuse = vector3(1.0f, 0.0f, 0.0f);
    ubo.numTriangles = numTriangles;
    ubo.numMaterials = numMaterials;
    ubo.numSecondaryRays = numSecondaryRays;
    ubo.numDiffuseRays = numDiffuseRays;
    ubo.Nx = width;
    ubo.Ny = height;
    ubo.xtot = distance;
    ubo.ytot = distance;
    ubo.boundingBoxDistance = diagonalDiff;
    ubo.timeSeed = time_seed;
    ubo.gpuSum = gpuSum;

    // get camera translation via its matrix
    Eigen::Vector4f camPosM = view.inverse().col(3);
    vector3 camPos = vector3(camPosM[0], camPosM[1], camPosM[2]);
    ubo.worldCamPos = camPos;

    updateUniforms(ubo);
    //recordComputeCommandBuffer(computeCommandBuffer);

    auto start = std::chrono::high_resolution_clock::now();
    dispatchCompute();
    waitForComputeWork();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    //std::cout << "Compute work took " << duration.count() << " microseconds" << std::endl;

    // sum did happen on the GPU, write back only single value and compute
    if (gpuSum) {
        writeBackSingleValue();
        float apix = distance/width * distance/height;
        double PS = PRESSURE;
        vec3 totalForce = vector3(forces[0].x, forces[0].y, forces[0].z);
        force = PS*apix/msat*(totalForce);

    }
    // write back whole buffer and sum on CPU
    else {
        writeBackCPU();
        sumForces(force);
    }

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

    uint32_t maxComputeWorkGroupX = this->physicalDeviceProps.m_Properties.limits.maxComputeWorkGroupCount[0];
    uint32_t maxComputeWorkGroupY = this->physicalDeviceProps.m_Properties.limits.maxComputeWorkGroupCount[1];
    uint32_t maxInvocations = this->physicalDeviceProps.m_Properties.limits.maxComputeWorkGroupInvocations;
    std::cout << "Maximum workgroup invocations: " << maxInvocations << std::endl;
    std::cout << "Max compute workgroup count: " << maxComputeWorkGroupX << "x" << maxComputeWorkGroupY << std::endl;

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

    // enable atomic add feature for buffers
    VkPhysicalDeviceShaderAtomicFloatFeaturesEXT floatFeatures{};
    floatFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT;
    floatFeatures.shaderBufferFloat32AtomicAdd = VK_TRUE;


    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    createInfo.pNext = &floatFeatures;  // chain additional features

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
    #define LAYOUT_BINDING_COUNT 4
    VkDescriptorSetLayoutBinding layoutBindings[LAYOUT_BINDING_COUNT];

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

    layoutBindings[2].binding = 2;
    layoutBindings[2].descriptorCount = 1;
    layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBindings[2].pImmutableSamplers = nullptr;
    layoutBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    layoutBindings[3].binding = 3;
    layoutBindings[3].descriptorCount = 1;
    layoutBindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBindings[3].pImmutableSamplers = nullptr;
    layoutBindings[3].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;


    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = LAYOUT_BINDING_COUNT;
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

    //std::vector<float> sampleData(DATA_COUNT, 0.0f);

    // =========CREATE AND UPLOAD TRIANGLES SSBO=================

    std::vector<cTriangle> triangles = prepareTriangles();

    VkDeviceSize bufferSize = numTriangles * sizeof(cTriangle);

    // Create a staging buffer used to upload data to the gpu
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, triangles.data(), (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    // Copy data to all storage buffers
    createBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, trianglesSSBO, trianglesSSBOMemory);
    copyBuffer(stagingBuffer, trianglesSSBO, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    // =========CREATE AND UPLOAD MATERIAL SSBO=================
    std::vector<cMaterial> materials = prepareMaterials();

    bufferSize =  numMaterials * sizeof(cMaterial);

    // Create a staging buffer used to upload data to the gpu
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, materials.data(), (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    // Copy data to all storage buffers
    createBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, materialsSSBO, materialsSSBOMemory);
    copyBuffer(stagingBuffer, materialsSSBO, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    // =========CPU WRITE BACK BUFFER=================
    bufferSize = width * height * sizeof(vector4);
    createBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, forcesSSBO, forcesSSBOMemory);

}

void ComputeGPU::createUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffer, uniformBufferMemory);

    vkMapMemory(device, uniformBufferMemory, 0, bufferSize, 0, &uniformBufferMapped);

}

void ComputeGPU::createDescriptorPool() {
    VkDescriptorPoolSize poolSizes[4];
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(1);

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(1);

    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[2].descriptorCount = static_cast<uint32_t>(1);

    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[2].descriptorCount = static_cast<uint32_t>(1);

    poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[3].descriptorCount = static_cast<uint32_t>(1);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 4;
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

    std::array<VkWriteDescriptorSet, 4> descriptorWrites{};
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = computeDescriptorSets;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &uniformBufferInfo;

    VkDescriptorBufferInfo storageBufferInfo{};
    storageBufferInfo.buffer = trianglesSSBO;
    storageBufferInfo.offset = 0;
    storageBufferInfo.range = sizeof(cTriangle) * numTriangles;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = computeDescriptorSets;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = &storageBufferInfo;

    VkDescriptorBufferInfo storageBufferInfo2{};
    storageBufferInfo2.buffer = materialsSSBO;
    storageBufferInfo2.offset = 0;
    storageBufferInfo2.range = sizeof(cMaterial) * numMaterials;

    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[2].dstSet = computeDescriptorSets;
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pBufferInfo = &storageBufferInfo2;

    VkDescriptorBufferInfo storageBufferInfo3{};
    storageBufferInfo3.buffer = forcesSSBO;
    storageBufferInfo3.offset = 0;
    storageBufferInfo3.range = sizeof(vector4) * width * height;

    descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[3].dstSet = computeDescriptorSets;
    descriptorWrites[3].dstBinding = 3;
    descriptorWrites[3].dstArrayElement = 0;
    descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrites[3].descriptorCount = 1;
    descriptorWrites[3].pBufferInfo = &storageBufferInfo3;

    vkUpdateDescriptorSets(device, 4, descriptorWrites.data(), 0, nullptr);

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

void ComputeGPU::updateUniforms() {

    camera.setViewport(0, 0, 512, 512);
    camera.setProjection(kFieldOfView, kZNear, kZFar);

    Eigen::Matrix4f view = camera.setView();

    this->light = new Light();

    int time_seed = getTimeSeed();


    TriangleMesh *mesh = this->satellite->getMesh();
    Eigen::Vector3f diff = mesh->max_-mesh->min_;
    float diagonalDiff = diff.norm();
    float errorMargin = 0.1f;
    this->distance = diagonalDiff+errorMargin;

    //float xAxis = distance/2.0f;
    //float yAxis = distance/2.0f;

    UniformBufferObject ubo{};
    ubo.lightDirection = light->getLightDir();
    ubo.V1 = light->getRightDir();
    ubo.V2 = light->getUpDir();
    ubo.model = camera.setModel();
    ubo.worldCamPos = vector3(0.0f, 0.0f, 0.0f);
    ubo.debugMode = 2;
    ubo.diffuse = vector3(1.0f, 0.0f, 0.0f);
    ubo.numTriangles = numTriangles;
    ubo.numMaterials = numMaterials;
    ubo.numSecondaryRays = numSecondaryRays;
    ubo.numDiffuseRays = numDiffuseRays;
    ubo.Nx = width;
    ubo.Ny = height;
    ubo.xtot = distance;
    ubo.ytot = distance;
    ubo.boundingBoxDistance = distance;
    ubo.timeSeed = time_seed;
    ubo.gpuSum = 0;

    // get camera translation via its matrix
    Eigen::Vector4f camPosM = view.inverse().col(3);
    vector3 camPos = vector3(camPosM[0], camPosM[1], camPosM[2]);
    ubo.worldCamPos = camPos;

    memcpy(uniformBufferMapped, &ubo, sizeof(ubo));
}

void ComputeGPU::updateUniforms(UniformBufferObject ubo) {

    memcpy(uniformBufferMapped, &ubo, sizeof(ubo));
}


int ComputeGPU::getTimeSeed() {
    // TIME SEED
    const auto now = std::chrono::system_clock::now();
    //const std::time_t t_c = std::chrono::system_clock::to_time_t(now);
    const auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
    const auto fraction = now - seconds;
    const auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(fraction).count();
    // http://en.cppreference.com/w/cpp/chrono/c/time
    const std::time_t currentNow = std::time(nullptr) ; // get the current time point
    // convert it to (local) calendar time
    // http://en.cppreference.com/w/cpp/chrono/c/localtime
    const std::tm calendarTime = *std::localtime( std::addressof(currentNow) ) ;
    const auto secs      = calendarTime.tm_sec;
    const auto mins      = calendarTime.tm_min;
    const auto hours     = calendarTime.tm_hour;
    return millisecs + 1000*(secs + 60*(mins + 60*(hours)));
}



void ComputeGPU::recordComputeCommandBuffer(VkCommandBuffer commandBuffer) {
    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording compute command buffer!");
    }

    if (gpuSum) {
        // initialize forces[0] to 0, for proper sum reduction
        VkMemoryBarrier memoryBarrierZero{};
        memoryBarrierZero.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        memoryBarrierZero.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        memoryBarrierZero.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

        vkCmdFillBuffer(commandBuffer, forcesSSBO, 0, 16, 0);
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &memoryBarrierZero, 0, nullptr, 0, nullptr);
    }


    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout, 0, 1, &computeDescriptorSets, 0, nullptr);

    // dispatch (width / 32) * (height / 32) work groups, with a local size
    // of 32x32, for a total of width * height individual invocations
    //vkCmdDispatch(commandBuffer, width / 32, height / 32, 1);

#define WORKGROUP_SIZE 8  // workgroup size for x and y dims, should be also changed on shader

    int x = std::ceil((float) width / WORKGROUP_SIZE);
    int y = std::ceil((float) height / WORKGROUP_SIZE);
    //std::cout << "Pixel grid: " << width << "x" << height << "\n";
    //std::cout << "Dispatching " << x << "x" << y << " workgroups\n";

    vkCmdDispatch(commandBuffer, x, y, 1);

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

    VkResult resetResult = vkResetCommandBuffer(computeCommandBuffer, 0);
    if(resetResult != VK_SUCCESS) {
        std::cerr << "vkResetCommandBuffer failed with error: " << vkResultToString(resetResult) << std::endl;
        throw std::runtime_error("failed to reset compute command buffer");
    }
    recordComputeCommandBuffer(computeCommandBuffer);

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &computeCommandBuffer;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;


    VkResult result = vkQueueSubmit(computeQueue, 1, &submitInfo, computeFence);
    if(result != VK_SUCCESS) {
        std::cerr << "vkQueueSubmit failed with error: " << vkResultToString(result) << std::endl;
        throw std::runtime_error("failed to submit compute command buffer");
    }

}

void ComputeGPU::waitForComputeWork() {
    // wait for compute work to finish
    VkResult waitResult = vkWaitForFences(device, 1, &computeFence, VK_TRUE, UINT64_MAX);
    if(waitResult != VK_SUCCESS) {
        std::cerr << "vkWaitForFences failed with error: " << vkResultToString(waitResult) << std::endl;
        throw std::runtime_error("failed to wait on compute fence");
    }
    // reset the fence for reuse
    vkResetFences(device, 1, &computeFence);
}

void ComputeGPU::writeBackCPU() {
    // Create a staging buffer used to retrieve data to the gpu
    // is this the best way to do it??
    VkDeviceSize bufferSize = width * height * sizeof(vector4);
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    copyBuffer(forcesSSBO, stagingBuffer, bufferSize);

    forces.resize(width * height);

    vector4* bufferData;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, (void **) &bufferData);
    memcpy(forces.data(), bufferData, (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void ComputeGPU::writeBackSingleValue() {
    VkDeviceSize bufferSize = sizeof(vector4);
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    copyBuffer(forcesSSBO, stagingBuffer, bufferSize);

    forces.resize(width * height);

    vector4* bufferData;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, (void **) &bufferData);
    memcpy(forces.data(), bufferData, (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void ComputeGPU::sumForces(vector3& force) {
    /*
    // print data
    for(uint32_t i = 0; i < width * height; i = i + 1) {
        std::cout << "at " << i << " " << forces[i] << " ";
    }
    */

    vector3 totalForce = vector3(0.0f,0.0f,0.0f);
    const uint32_t n_pixels = width * height;
    for(uint32_t i = 0; i < n_pixels; i++) {
        totalForce += vector3(forces[i].x, forces[i].y, forces[i].z);
    }

    float apix = distance/width * distance/height;
    double PS = PRESSURE;
    force = PS*apix/msat*(totalForce);

}


void ComputeGPU::cleanup() {

    vkDestroyPipeline(device, computePipeline, nullptr);
    vkDestroyPipelineLayout(device, computePipelineLayout, nullptr);



    vkDestroyBuffer(device, uniformBuffer, nullptr);
    vkFreeMemory(device, uniformBufferMemory, nullptr);

    vkDestroyDescriptorPool(device, descriptorPool, nullptr);

    vkDestroyDescriptorSetLayout(device, computeDescriptorSetLayout, nullptr);

    vkDestroyBuffer(device, trianglesSSBO, nullptr);
    vkFreeMemory(device, trianglesSSBOMemory, nullptr);

    vkDestroyBuffer(device, materialsSSBO, nullptr);
    vkFreeMemory(device, materialsSSBOMemory, nullptr);

    vkDestroyBuffer(device, forcesSSBO, nullptr);
    vkFreeMemory(device, forcesSSBOMemory, nullptr);


    vkDestroyFence(device, computeFence, nullptr);

    vkDestroyCommandPool(device, commandPool, nullptr);

    vkDestroyDevice(device, nullptr);

}

ComputeGPU::~ComputeGPU() {
    cleanup();
}




