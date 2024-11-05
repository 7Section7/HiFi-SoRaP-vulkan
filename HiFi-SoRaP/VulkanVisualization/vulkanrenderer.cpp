#include "vulkanrenderer.h"
#include <QVulkanFunctions>
#include <QFile>


// Note that the vertex data and the projection matrix assume OpenGL. With
// Vulkan Y is negated in clip space and the near/far plane is at 0/1 instead
// of -1/1. These will be corrected for by an extra transformation when
// calculating the modelview-projection matrix.
/*
static float vertexData[] = { // Y up, front = CCW
    0.0f,   0.0f,   1.0f, 0.0f, 0.0f,
    -1.0f,  -1.0f,   0.0f, 1.0f, 0.0f,
    1.0f,  -1.0f,   0.0f, 0.0f, 1.0f
};
*/


struct Vertex {
    float position[4];
    float normal[4];
};


static const int UNIFORM_DATA_SIZE = 16 * sizeof(float);

static inline VkDeviceSize aligned(VkDeviceSize v, VkDeviceSize byteAlign)
{
    return (v + byteAlign - 1) & ~(byteAlign - 1);
}

/**
 * @brief VulkanRenderer::createShader
 * @param name Compiled .spv shader file
 * @return VkShaderModule
 */
VkShaderModule VulkanRenderer::createShader(const QString &name)
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
    VkResult err = m_devFuncs->vkCreateShaderModule(m_window->device(), &shaderInfo, nullptr, &shaderModule);
    if (err != VK_SUCCESS) {
        qWarning("Failed to create shader module: %d", err);
        return VK_NULL_HANDLE;
    }

    return shaderModule;
}

VulkanRenderer::VulkanRenderer(VulkanWindow* w)
    : m_window(w)
{}

void VulkanRenderer::initResources() {
    qDebug("initResources");

    QVulkanInstance* inst = m_window->vulkanInstance();
    VkDevice dev = m_window->device();

    m_devFuncs = inst->deviceFunctions(dev);

    // Prepare the vertex and uniform data. The vertex data will never
    // change so one buffer is sufficient regardless of the value of
    // QVulkanWindow::CONCURRENT_FRAME_COUNT. Uniform data is changing per
    // frame however so active frames have to have a dedicated copy.

    // Use just one memory allocation and one buffer. We will then specify the
    // appropriate offsets for uniform buffers in the VkDescriptorBufferInfo.
    // Have to watch out for
    // VkPhysicalDeviceLimits::minUniformBufferOffsetAlignment, though.

    // The uniform buffer is not strictly required in this example, we could
    // have used push constants as well since our single matrix (64 bytes) fits
    // into the spec mandated minimum limit of 128 bytes. However, once that
    // limit is not sufficient, the per-frame buffers, as shown below, will
    // become necessary.

    const int concurrentFrameCount = m_window->concurrentFrameCount();
    const VkPhysicalDeviceLimits* pDevLimits = &m_window->physicalDeviceProperties()->limits;
    const VkDeviceSize uniAlign = pDevLimits->minUniformBufferOffsetAlignment;
    qDebug("uniform buffer offset aligment is %u", (uint) uniAlign);

    // TODO: take precision into account
    typedef precision::value_type VertexFloat;

    quint8 *vertData = reinterpret_cast<quint8*>(
        m_window->satellite->getMesh()->replicatedVertices.data()
    );
    const int vertexByteSize = m_window->satellite->getMesh()->replicatedVertices.size() * sizeof(vector4);

    quint8* normalsData = reinterpret_cast<quint8*>(
        m_window->satellite->getMesh()->replicatedNormals.data()
    );
    const int normalsByteSize = m_window->satellite->getMesh()->replicatedNormals.size() * sizeof(vector4);

    const VkDeviceSize vertexDataSize = vertexByteSize + normalsByteSize;


    // create buffer to store vertices
    VkBufferCreateInfo bufInfo;
    memset(&bufInfo, 0, sizeof(bufInfo));

    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    // Our internal layout is vertex, uniform, uniform, ... with each uniform buffer start offset aligned to uniAlign.
    //const VkDeviceSize vertexAllocSize = aligned(sizeof(vertexData), uniAlign);
    const VkDeviceSize vertexAllocSize = aligned(vertexDataSize, uniAlign);
    const VkDeviceSize uniformAllocSize = aligned(UNIFORM_DATA_SIZE, uniAlign);
    bufInfo.size = vertexAllocSize + concurrentFrameCount * uniformAllocSize;
    // use as both vertex buffer and uniform buffer
    bufInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    VkResult err = m_devFuncs->vkCreateBuffer(dev, &bufInfo, nullptr, &m_buf);
    if (err != VK_SUCCESS) {
        qFatal("Failed to create buffer: %d", err);
    }

    VkMemoryRequirements memReq;
    m_devFuncs->vkGetBufferMemoryRequirements(dev, m_buf, &memReq);

    VkMemoryAllocateInfo memAllocInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        nullptr,
        memReq.size,
        m_window->hostVisibleMemoryIndex()
    };

    // allocate device (GPU) memory
    err = m_devFuncs->vkAllocateMemory(dev, &memAllocInfo, nullptr, &m_bufMem);
    if (err != VK_SUCCESS)
        qFatal("Failed to allocate memory: %d", err);

    // bind the allocated memory to the just created buffer
    err = m_devFuncs->vkBindBufferMemory(dev, m_buf, m_bufMem, 0);
    if (err != VK_SUCCESS)
        qFatal("Failed to bind buffer memory: %d", err);

    // map device memory into application address space to fill the buffer
    quint8 *p;
    err = m_devFuncs->vkMapMemory(dev, m_bufMem, 0, memReq.size, 0, reinterpret_cast<void **>(&p));
    if (err != VK_SUCCESS)
        qFatal("Failed to map memory: %d", err);

    memcpy(p, vertData, vertexByteSize);
    memcpy(p + vertexByteSize, normalsData, normalsByteSize);

    //memcpy(p + vertexByte, vertexData, sizeof(vertexData));

    QMatrix4x4 ident;
    memset(m_uniformBufInfo, 0, sizeof(m_uniformBufInfo));
    for (int i = 0; i < concurrentFrameCount; ++i) {
        const VkDeviceSize offset = vertexAllocSize + i * uniformAllocSize;
        memcpy(p + offset, ident.constData(), 16 * sizeof(float));
        m_uniformBufInfo[i].buffer = m_buf;
        m_uniformBufInfo[i].offset = offset;
        m_uniformBufInfo[i].range = uniformAllocSize;
    }
    m_devFuncs->vkUnmapMemory(dev, m_bufMem);

    // VERTEX ATTRIBUTES

    // for interleaved data
    /*
    VkVertexInputBindingDescription vertexBindingDesc{};
    vertexBindingDesc.binding = 0;
    vertexBindingDesc.stride = sizeof(Vertex);
    vertexBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;


    VkVertexInputAttributeDescription vertexAttrDesc[2];

    vertexAttrDesc[0].binding = 0;
    vertexAttrDesc[0].location = 0;
    vertexAttrDesc[0].format = VK_FORMAT_R32G32_SFLOAT;
    vertexAttrDesc[0].offset = offsetof(Vertex, position);

    vertexAttrDesc[1].binding = 0;
    vertexAttrDesc[1].location = 1;
    vertexAttrDesc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexAttrDesc[1].offset = offsetof(Vertex, normal);
    */

    // for non-interleaved data
    VkVertexInputBindingDescription vertexBindingDesc[2];
    vertexBindingDesc[0].binding = 0;
    vertexBindingDesc[0].stride = 4 * sizeof(float);
    vertexBindingDesc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    vertexBindingDesc[1].binding = 1;
    vertexBindingDesc[0].stride = 4 * sizeof(float);
    vertexBindingDesc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;


    VkVertexInputAttributeDescription vertexAttrDesc[2];

    vertexAttrDesc[0].binding = 0;
    vertexAttrDesc[0].location = 0;
    vertexAttrDesc[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    vertexAttrDesc[0].offset = 0;

    vertexAttrDesc[1].binding = 1;
    vertexAttrDesc[1].location = 1;
    vertexAttrDesc[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    vertexAttrDesc[1].offset = 0;

    // vertex input
    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.pNext = nullptr;
    vertexInputInfo.flags = 0;
    vertexInputInfo.vertexBindingDescriptionCount = 2;
    vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDesc[0];
    vertexInputInfo.vertexAttributeDescriptionCount = 2;
    vertexInputInfo.pVertexAttributeDescriptions = &vertexAttrDesc[0];

    // Set up descriptor set and its layout.
    VkDescriptorPoolSize descPoolSizes = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uint32_t(concurrentFrameCount) };
    VkDescriptorPoolCreateInfo descPoolInfo;
    memset(&descPoolInfo, 0, sizeof(descPoolInfo));
    descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descPoolInfo.maxSets = concurrentFrameCount;
    descPoolInfo.poolSizeCount = 1;
    descPoolInfo.pPoolSizes = &descPoolSizes;
    err = m_devFuncs->vkCreateDescriptorPool(dev, &descPoolInfo, nullptr, &m_descPool);
    if (err != VK_SUCCESS)
        qFatal("Failed to create descriptor pool: %d", err);


    VkDescriptorSetLayoutBinding layoutBinding = {
        0, // binding
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1,
        VK_SHADER_STAGE_VERTEX_BIT,
        nullptr
    };
    VkDescriptorSetLayoutCreateInfo descLayoutInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        nullptr,
        0,
        1,
        &layoutBinding
    };
    err = m_devFuncs->vkCreateDescriptorSetLayout(dev, &descLayoutInfo, nullptr, &m_descSetLayout);
    if (err != VK_SUCCESS)
        qFatal("Failed to create descriptor set layout: %d", err);

    for (int i = 0; i < concurrentFrameCount; ++i) {
        VkDescriptorSetAllocateInfo descSetAllocInfo = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            nullptr,
            m_descPool,
            1,
            &m_descSetLayout
        };
        err = m_devFuncs->vkAllocateDescriptorSets(dev, &descSetAllocInfo, &m_descSet[i]);
        if (err != VK_SUCCESS)
            qFatal("Failed to allocate descriptor set: %d", err);

        VkWriteDescriptorSet descWrite;
        memset(&descWrite, 0, sizeof(descWrite));
        descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrite.dstSet = m_descSet[i];
        descWrite.descriptorCount = 1;
        descWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descWrite.pBufferInfo = &m_uniformBufInfo[i];
        m_devFuncs->vkUpdateDescriptorSets(dev, 1, &descWrite, 0, nullptr);
    }

    // Pipeline cache
    VkPipelineCacheCreateInfo pipelineCacheInfo;
    memset(&pipelineCacheInfo, 0, sizeof(pipelineCacheInfo));
    pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    err = m_devFuncs->vkCreatePipelineCache(dev, &pipelineCacheInfo, nullptr, &m_pipelineCache);
    if (err != VK_SUCCESS)
        qFatal("Failed to create pipeline cache: %d", err);

    // Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo;
    memset(&pipelineLayoutInfo, 0, sizeof(pipelineLayoutInfo));
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_descSetLayout;
    err = m_devFuncs->vkCreatePipelineLayout(dev, &pipelineLayoutInfo, nullptr, &m_pipelineLayout);
    if (err != VK_SUCCESS)
        qFatal("Failed to create pipeline layout: %d", err);


    // Shaders
    VkShaderModule vertShaderModule = createShader(QStringLiteral(":/resources/shaders/color_vert.spv"));
    VkShaderModule fragShaderModule = createShader(QStringLiteral(":/resources/shaders/color_frag.spv"));

    qInfo("Shaders successfully created");

    // Graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo;
    memset(&pipelineInfo, 0, sizeof(pipelineInfo));
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    VkPipelineShaderStageCreateInfo shaderStages[2] = {
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            VK_SHADER_STAGE_VERTEX_BIT,
            vertShaderModule,
            "main",
            nullptr
        },
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            fragShaderModule,
            "main",
            nullptr
        }
    };
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    pipelineInfo.pVertexInputState = &vertexInputInfo;

    // input assembly
    VkPipelineInputAssemblyStateCreateInfo ia;
    memset(&ia, 0, sizeof(ia));
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineInfo.pInputAssemblyState = &ia;

    // The viewport and scissor will be set dynamically via vkCmdSetViewport/Scissor.
    // This way the pipeline does not need to be touched when resizing the window.
    VkPipelineViewportStateCreateInfo vp;
    memset(&vp, 0, sizeof(vp));
    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.viewportCount = 1;
    vp.scissorCount = 1;
    pipelineInfo.pViewportState = &vp;

    // rasterization
    VkPipelineRasterizationStateCreateInfo rs;
    memset(&rs, 0, sizeof(rs));
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_NONE; // we want the back face as well
    rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rs.lineWidth = 1.0f;
    pipelineInfo.pRasterizationState = &rs;

    VkPipelineMultisampleStateCreateInfo ms;
    memset(&ms, 0, sizeof(ms));
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    // Enable multisampling.
    ms.rasterizationSamples = m_window->sampleCountFlagBits();
    pipelineInfo.pMultisampleState = &ms;

    VkPipelineDepthStencilStateCreateInfo ds;
    memset(&ds, 0, sizeof(ds));
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.depthTestEnable = VK_TRUE;
    ds.depthWriteEnable = VK_TRUE;
    ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    pipelineInfo.pDepthStencilState = &ds;

    VkPipelineColorBlendStateCreateInfo cb;
    memset(&cb, 0, sizeof(cb));
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    // no blend, write out all of rgba
    VkPipelineColorBlendAttachmentState att;
    memset(&att, 0, sizeof(att));
    att.colorWriteMask = 0xF;
    cb.attachmentCount = 1;
    cb.pAttachments = &att;
    pipelineInfo.pColorBlendState = &cb;

    VkDynamicState dynEnable[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dyn;
    memset(&dyn, 0, sizeof(dyn));
    dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dyn.dynamicStateCount = sizeof(dynEnable) / sizeof(VkDynamicState);
    dyn.pDynamicStates = dynEnable;
    pipelineInfo.pDynamicState = &dyn;

    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_window->defaultRenderPass();


    err = m_devFuncs->vkCreateGraphicsPipelines(dev, m_pipelineCache, 1, &pipelineInfo, nullptr, &m_pipeline);
    if (err != VK_SUCCESS)
        qFatal("Failed to create graphics pipeline: %d", err);

    if (vertShaderModule)
        m_devFuncs->vkDestroyShaderModule(dev, vertShaderModule, nullptr);
    if (fragShaderModule)
        m_devFuncs->vkDestroyShaderModule(dev, fragShaderModule, nullptr);

}

void VulkanRenderer::initSwapChainResources() {
    qDebug("initSwapChainResources");

    // Projection matrix
    m_proj = m_window->clipCorrectionMatrix(); // adjust for Vulkan-OpenGL clip space differences
    const QSize sz = m_window->swapChainImageSize();
    m_proj.perspective(45.0f, sz.width() / (float) sz.height(), 0.01f, 100.0f);
    m_proj.translate(0, 0, -10);
}

void VulkanRenderer::releaseSwapChainResources() {
    qDebug("releaseSwapChainResources");
}

void VulkanRenderer::releaseResources() {
    qDebug("releaseResources");

    VkDevice dev = m_window->device();

    if (m_pipeline) {
        m_devFuncs->vkDestroyPipeline(dev, m_pipeline, nullptr);
        m_pipeline = VK_NULL_HANDLE;
    }

    if (m_pipelineLayout) {
        m_devFuncs->vkDestroyPipelineLayout(dev, m_pipelineLayout, nullptr);
        m_pipelineLayout = VK_NULL_HANDLE;
    }

    if (m_pipelineCache) {
        m_devFuncs->vkDestroyPipelineCache(dev, m_pipelineCache, nullptr);
        m_pipelineCache = VK_NULL_HANDLE;
    }

    if (m_descSetLayout) {
        m_devFuncs->vkDestroyDescriptorSetLayout(dev, m_descSetLayout, nullptr);
        m_descSetLayout = VK_NULL_HANDLE;
    }

    if (m_descPool) {
        m_devFuncs->vkDestroyDescriptorPool(dev, m_descPool, nullptr);
        m_descPool = VK_NULL_HANDLE;
    }

    if (m_buf) {
        m_devFuncs->vkDestroyBuffer(dev, m_buf, nullptr);
        m_buf = VK_NULL_HANDLE;
    }

    if (m_bufMem) {
        m_devFuncs->vkFreeMemory(dev, m_bufMem, nullptr);
        m_bufMem = VK_NULL_HANDLE;
    }
}

void VulkanRenderer::startNextFrame() {

    // we will be adding commands to the currentCommandBuffer managed by QVulkanWindow
    VkDevice dev = m_window->device();
    VkCommandBuffer cb = m_window->currentCommandBuffer();
    const QSize sz = m_window->swapChainImageSize();

    VkClearColorValue clearColor = {{ 0, 0, 0, 1 }};
    VkClearDepthStencilValue clearDS = { 1, 0 };
    VkClearValue clearValues[3];
    memset(clearValues, 0, sizeof(clearValues));
    clearValues[0].color = clearValues[2].color = clearColor;
    clearValues[1].depthStencil = clearDS;

    VkRenderPassBeginInfo rpBeginInfo;
    memset(&rpBeginInfo, 0, sizeof(rpBeginInfo));
    rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBeginInfo.renderPass = m_window->defaultRenderPass();
    rpBeginInfo.framebuffer = m_window->currentFramebuffer();
    rpBeginInfo.renderArea.extent.width = sz.width();
    rpBeginInfo.renderArea.extent.height = sz.height();
    rpBeginInfo.clearValueCount = m_window->sampleCountFlagBits() > VK_SAMPLE_COUNT_1_BIT ? 3 : 2;
    rpBeginInfo.pClearValues = clearValues;
    VkCommandBuffer cmdBuf = m_window->currentCommandBuffer();
    m_devFuncs->vkCmdBeginRenderPass(cmdBuf, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    quint8 *p;
    VkResult err = m_devFuncs->vkMapMemory(dev, m_bufMem, m_uniformBufInfo[m_window->currentFrame()].offset,
                                           UNIFORM_DATA_SIZE, 0, reinterpret_cast<void **>(&p));
    if (err != VK_SUCCESS)
        qFatal("Failed to map memory: %d", err);
    QMatrix4x4 m = m_proj;
    m.rotate(m_rotation, 0, 1, 0);
    memcpy(p, m.constData(), 16 * sizeof(float));
    m_devFuncs->vkUnmapMemory(dev, m_bufMem);

    // Not exactly a real animation system, just advance on every frame for now.
    m_rotation += 1.0f;

    m_devFuncs->vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    m_devFuncs->vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1,
                                        &m_descSet[m_window->currentFrame()], 0, nullptr);
    VkDeviceSize vbOffset = 0;
    m_devFuncs->vkCmdBindVertexBuffers(cb, 0, 1, &m_buf, &vbOffset);
    m_devFuncs->vkCmdBindVertexBuffers(cb, 1, 1, &m_buf, &vbOffset);

    VkViewport viewport;
    viewport.x = viewport.y = 0;
    viewport.width = sz.width();
    viewport.height = sz.height();
    viewport.minDepth = 0;
    viewport.maxDepth = 1;
    m_devFuncs->vkCmdSetViewport(cb, 0, 1, &viewport);

    // scissor matches the viewport size
    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = viewport.width;
    scissor.extent.height = viewport.height;
    m_devFuncs->vkCmdSetScissor(cb, 0, 1, &scissor);

    uint32_t numVertices = static_cast<uint32_t>(
        m_window->satellite->getMesh()->replicatedVertices.size()
    );
    m_devFuncs->vkCmdDraw(cb, numVertices, 1, 0, 0);

    m_devFuncs->vkCmdEndRenderPass(cmdBuf);

    m_window->frameReady();
    m_window->requestUpdate(); // render continuously, throttled by the presentation rate

}
