#ifndef COMPUTEGPU_H
#define COMPUTEGPU_H

#include <vulkan/vulkan.h>  // vulkan header must be before SRP
#include "GLVisualization/light.h"
#include "SRP/advancedsrp.h"
#include "GLVisualization/camera.h"


const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
};

const QString computeShaderFilename = ":/resources/shaders/test_comp.spv";


const bool enableValidationLayers = true;


// structs
struct cTriangle {
    vector3 v1;
    vector3 v2;
    vector3 v3;
    int materialIndex;
};

struct cMaterial {
    float ps;
    float pd;
    float refIdx;
    uint32_t reflectiveness;
};


class ComputeGPU : public AdvancedSRP
{

    struct PhysicalDeviceProps
    {
        VkPhysicalDeviceProperties              m_Properties;
        VkPhysicalDeviceFeatures                m_Features;
        VkPhysicalDeviceMemoryProperties        m_MemoryProperties;
        std::vector<VkQueueFamilyProperties>    m_QueueFamilyProperties;
        std::vector<VkLayerProperties>          m_LayerProperties;
        std::vector<VkExtensionProperties>      m_ExtensionProperties;
    };

    VkInstance instance;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    PhysicalDeviceProps physicalDeviceProps;
    VkDevice device;

    VkQueue computeQueue;

    VkDescriptorSetLayout computeDescriptorSetLayout;
    VkPipelineLayout computePipelineLayout;
    VkPipeline computePipeline;

    VkCommandPool commandPool;

    VkBuffer trianglesSSBO;
    VkDeviceMemory trianglesSSBOMemory;

    VkBuffer materialsSSBO;
    VkDeviceMemory materialsSSBOMemory;

    VkBuffer forcesSSBO;
    VkDeviceMemory forcesSSBOMemory;


    VkBuffer uniformBuffer;
    VkDeviceMemory uniformBufferMemory;
    void* uniformBufferMapped;

    VkDescriptorPool descriptorPool;
    VkDescriptorSet computeDescriptorSets;

    VkCommandBuffer computeCommandBuffer;

    VkFence computeFence;

public:
    ComputeGPU(VkInstance instance);

    void pickPhysicalDevice();
    uint32_t findQueueFamilies();
    void createLogicalDevice();
    void createComputeDescriptorSetLayout();

    VkShaderModule createShaderModule(const QString &name);
    void createComputePipeline();
    void createCommandPool();
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void createShaderStorageBuffers();
    void createUniformBuffers();
    void createDescriptorPool();
    void createComputeDescriptorSets();

    void createComputeCommandBuffers();

    void createSyncObjects();

    void updateUniforms();
    void recordComputeCommandBuffer(VkCommandBuffer commandBuffer);
    void dispatchCompute();

    void waitForComputeWork();
    void writeBackCPU();

    void init();
    void process();
    void cleanup();


    void computeStepSRP(const vector3& XS, vector3& force, const vector3& V1 = DEFAULT_VEC3,
                        const vector3& V2 = DEFAULT_VEC3) {};

    std::vector<cTriangle> prepareTriangles();
    std::vector<cMaterial> prepareMaterials();



public:

    dataVisualization::Camera camera;

    uint32_t width, height;
    //Object *satellite;
    Light *light;

    uint32_t numTriangles;
    uint32_t numMaterials;
    uint32_t numSecondaryRays;
    uint32_t numDiffuseRays;
};

#endif // COMPUTEGPU_H
