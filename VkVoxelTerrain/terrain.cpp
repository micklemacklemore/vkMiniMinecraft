#include "terrain.h"
#include "vulkan_setup.h"
#include "types.h"
#include "renderer.h"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>

// a "zone" is a 4*4 area of chunks (64 * 64 blocks)
// a "chunk" contains 16 * 256 * 16 blocks
#define TERRAIN_DRAW_MULTIPLIER 1       // (Default: 1 / 3x3 draw zone radius) 
#define TERRAIN_CREATE_MULTIPLIER 1     // (Default: 2 / 5x5 create zone radius) 
#define ZONE_SIZE 64                    // the length of a zone (in blocks) 
#define CHUNK_LENGTH 16                 // chunk length/width

#define TERRAIN_DRAW_RADIUS         ZONE_SIZE * TERRAIN_DRAW_MULTIPLIER
#define TERRAIN_CREATE_RADIUS       ZONE_SIZE * TERRAIN_CREATE_MULTIPLIER

int roundDown(int n, int m) {
    return n >= 0 ? (n / m) * m : ((n - m + 1) / m) * m;
}

Terrain::Terrain(Renderer* vulkanContext)
    : context(vulkanContext), m_chunks(), m_chunks_mutex(), m_generatedTerrain(), pipelineChunks(VK_NULL_HANDLE),
    descriptorSetLayout(VK_NULL_HANDLE), pipelineLayout(VK_NULL_HANDLE), currentPipeline(nullptr),
    threadPool(16), pendingChunks(), pendingChunksMutex(), drawableChunks(), drawableChunksMutex(),
    transferCmdPoolManager{}
{}

Terrain::~Terrain() {
}

void Terrain::buildPipelines()
{
    // Create descriptor set layout
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.descriptorCount = 1;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(context->device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    // Shader Modules
    auto vertShaderCode = readFile("shaders/vert_chunked.spv");
    auto fragShaderCode = readFile("shaders/frag_chunked.spv");

    VkShaderModule vertShaderModule = createShaderModule(context->device, vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(context->device, fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    // Vertex Input
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    // Configure + Enable Depth Testing
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional

    // Setup dynamic resizing of viewport
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // Create Pipeline Layout
    //VkPushConstantRange pushConstantRange{};
    //pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // or fragment if needed
    //pushConstantRange.offset = 0;
    //pushConstantRange.size = sizeof(glm::mat4);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    //pipelineLayoutInfo.pushConstantRangeCount = 1;
    //pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(context->device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = context->renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(context->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelineChunks) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(context->device, fragShaderModule, nullptr);
    vkDestroyShaderModule(context->device, vertShaderModule, nullptr);

    currentPipeline = &pipelineChunks;

    // init the command pool manager

    QueueFamilyIndices indices = findQueueFamilies(context->physicalDevice, context->surface);
    transferCmdPoolManager.init(context->device, indices.transferFamily.value());
}

void Terrain::destroyResources()
{
    threadPool.destroy();
    transferCmdPoolManager.cleanup(); 
    vkDestroyDescriptorSetLayout(context->device, descriptorSetLayout, nullptr);

    vkDestroyPipeline(context->device, pipelineChunks, nullptr);
    vkDestroyPipelineLayout(context->device, pipelineLayout, nullptr);

    for (const auto& pair : m_chunks) {
        const uPtr<Chunk>& chunk = pair.second;
        if (chunk)
        {
            vkDestroyBuffer(context->device, chunk->VertexBuffer, nullptr);
            vkFreeMemory(context->device, chunk->VertexBufferMemory, nullptr);
        }
    }
}

// Surround calls to this with try-catch if you don't know whether
// the coordinates at x, y, z have a corresponding Chunk
BlockType Terrain::getBlockAt(int x, int y, int z)
{
    if (hasChunkAt(x, z)) {
        // Just disallow action below or above min/max height,
        // but don't crash the game over it.
        if (y < 0 || y >= 256) {
            return EMPTY;
        }
        const uPtr<Chunk>& c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        return c->getBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
            static_cast<unsigned int>(y),
            static_cast<unsigned int>(z - chunkOrigin.y));
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
            " " + std::to_string(y) + " " +
            std::to_string(z) + " have no Chunk!");
    }
}

bool Terrain::hasChunkAt(int x, int z) {
    // Map x and z to their nearest Chunk corner
    // By flooring x and z, then multiplying by 16,
    // we clamp (x, z) to its nearest Chunk-space corner,
    // then scale back to a world-space location.
    // Note that floor() lets us handle negative numbers
    // correctly, as floor(-1 / 16.f) gives us -1, as
    // opposed to (int)(-1 / 16.f) giving us 0 (incorrect!).
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.find(toKey(16 * xFloor, 16 * zFloor)) != m_chunks.end();
}

// Combine two 32-bit ints into one 64-bit int
// where the upper 32 bits are X and the lower 32 bits are Z
int64_t toKey(int x, int z) {
    int64_t xz = 0xffffffffffffffff;
    int64_t x64 = x;
    int64_t z64 = z;

    // Set all lower 32 bits to 1 so we can & with Z later
    xz = (xz & (x64 << 32)) | 0x00000000ffffffff;

    // Set all upper 32 bits to 1 so we can & with XZ
    z64 = z64 | 0xffffffff00000000;

    // Combine
    xz = xz & z64;
    return xz;
}

glm::ivec2 toCoords(int64_t k) {
    // Z is lower 32 bits
    int64_t z = k & 0x00000000ffffffff;
    // If the most significant bit of Z is 1, then it's a negative number
    // so we have to set all the upper 32 bits to 1.
    // Note the 8    V
    if(z & 0x0000000080000000) {
        z = z | 0xffffffff00000000;
    }
    int64_t x = (k >> 32);

    return glm::ivec2(x, z);
}

uPtr<Chunk>& Terrain::getChunkAt(int x, int z) {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    std::lock_guard<std::mutex> lock{ m_chunks_mutex }; 
    return m_chunks[toKey(16 * xFloor, 16 * zFloor)];
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    std::lock_guard<std::mutex> lock{ m_chunks_mutex };
    if(hasChunkAt(x, z)) {
        uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        c->setBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                      static_cast<unsigned int>(y),
                      static_cast<unsigned int>(z - chunkOrigin.y),
                      t);
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

int Terrain::generateTerrain(glm::ivec2 worldPos) {
    return 0; 
}

void Terrain::threadCreateBlockData(glm::vec2 terrainCoord)
{

    for (int z = terrainCoord[1]; z < terrainCoord[1] + ZONE_SIZE; z += 16) {
        for (int x = terrainCoord[0]; x < terrainCoord[0] + ZONE_SIZE; x += 16) {
            Chunk* chunk = instantiateChunkAt(x, z);

            // flat terrain
            for (int chunkX = 0; chunkX < 16; chunkX++) {
                for (int chunkZ = 0; chunkZ < 16; chunkZ++) {
                    glm::ivec2 worldPos(x + chunkX, z + chunkZ); 
                    if (glm::abs(worldPos.x) % 64 == 0 || glm::abs(worldPos.y) % 64 == 0) {
                        chunk->setBlockAt(chunkX, 128, chunkZ, STONE);
                    }
                    else {
                        chunk->setBlockAt(chunkX, 128, chunkZ, GRASS);
                    }
                    
                }
            }

            std::lock_guard<std::mutex> lock(pendingChunksMutex);
            pendingChunks.push_back(chunk); 
        }
    }
}

void Terrain::threadCreateBufferData(Chunk* chunk)
{
    chunk->createVertexData();
    std::lock_guard<std::mutex> lock(drawableChunksMutex);
    drawableChunks.push_back(chunk); 
}

void Terrain::tryExpansion(const glm::vec3& pos)
{
    int terrainX = roundDown(int(pos.x), ZONE_SIZE); 
    int terrainZ = roundDown(int(pos.z), ZONE_SIZE); 

    // the "create radius" are the collection of zones around the player with generated block data
    // the "draw radius" are the zones that are actually drawn to screen, which must be <= the create radius

    // check "create" radius around player, and spawn threads for zones without block data
    for (int z = terrainZ - TERRAIN_CREATE_RADIUS; z <= terrainZ + TERRAIN_CREATE_RADIUS; z += ZONE_SIZE) {
        for (int x = terrainX - TERRAIN_CREATE_RADIUS; x <= terrainX + TERRAIN_CREATE_RADIUS; x += ZONE_SIZE) {       // loop through each zone
            if (m_generatedTerrain.count(toKey(x, z)) == 0)
            {
                m_generatedTerrain.insert(toKey(x, z));
                threadPool.enqueue(&Terrain::threadCreateBlockData, this, glm::vec2(x, z));
            }
        }
    }

    std::vector<Chunk*> chunksToProcess;

    {
        std::lock_guard<std::mutex> lock(pendingChunksMutex);
        chunksToProcess.swap(pendingChunks); // Efficient: avoids copying
    }

    // TODO: I don't think this works: 
    //
    // you need to wait for the vk buffers to be filled before we can actually draw them, and we don't do that here. 
    // also, we need mutexes around queue submission, OR, what we should be doing is record all the cmd buffers and
    // submit all at once, or something
    for (Chunk* chunk : chunksToProcess) {
        threadPool.enqueue(&Terrain::threadCreateBufferData, this, chunk);
    }

    std::vector<Chunk*> copyChunks;

    {
        std::lock_guard<std::mutex> lock(drawableChunksMutex);
        copyChunks.swap(drawableChunks); // Efficient: avoids copying
    }

    for (Chunk* chunk : copyChunks)
    {
        chunk->createVkBuffer(context->device, context->physicalDevice,
            context->surface, context->commandPoolTransfer, context->queueTransfer);
    }
}

Chunk* Terrain::instantiateChunkAt(int x, int z) {
    uPtr<Chunk> chunk = mkU<Chunk>(x, z);
    Chunk *cPtr = chunk.get();
    std::lock_guard<std::mutex> lock{ m_chunks_mutex }; 
    m_chunks[toKey(x, z)] = move(chunk); 
    // Set the neighbor pointers of itself and its neighbors
    if(hasChunkAt(x, z + 16)) {
        auto &chunkNorth = m_chunks[toKey(x, z + 16)];
        cPtr->linkNeighbor(chunkNorth, ZPOS);
    }
    if(hasChunkAt(x, z - 16)) {
        auto &chunkSouth = m_chunks[toKey(x, z - 16)];
        cPtr->linkNeighbor(chunkSouth, ZNEG);
    }
    if(hasChunkAt(x + 16, z)) {
        auto &chunkEast = m_chunks[toKey(x + 16, z)];
        cPtr->linkNeighbor(chunkEast, XPOS);
    }
    if(hasChunkAt(x - 16, z)) {
        auto &chunkWest = m_chunks[toKey(x - 16, z)];
        cPtr->linkNeighbor(chunkWest, XNEG);
    }
    return cPtr;
}

void Terrain::drawZone(glm::ivec2 zone, VkCommandBuffer cmdBuffer, VkDescriptorSet descriptorSet) {
    for (int z = zone[1]; z < zone[1] + ZONE_SIZE; z += 16) {
        for (int x = zone[0]; x < zone[0] + ZONE_SIZE; x += 16) {
            const uPtr<Chunk>& chunk = getChunkAt(x, z);
            if (chunk && chunk->VertexBuffer != VK_NULL_HANDLE) {
                VkBuffer vertexBuffers[] = { chunk->VertexBuffer };
                VkDeviceSize offsets[] = { 0 };
                vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);
                vkCmdBindIndexBuffer(cmdBuffer, chunk->VertexBuffer, static_cast<VkDeviceSize>(sizeof(Vertex) * chunk->vertexSize), VK_INDEX_TYPE_UINT32);
                vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
                vkCmdDrawIndexed(cmdBuffer, chunk->numIndices, 1, 0, 0, 0);
            }
        }
    }
}


void Terrain::draw(const glm::vec3& position, VkCommandBuffer cmdBuffer, VkDescriptorSet descriptorSet) {
    int tx = roundDown(int(position.x), ZONE_SIZE);
    int tz = roundDown(int(position.z), ZONE_SIZE);

    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *currentPipeline);

    for (int z = tz - TERRAIN_DRAW_RADIUS; z <= tz + TERRAIN_DRAW_RADIUS; z += ZONE_SIZE) {
        for (int x = tx - TERRAIN_DRAW_RADIUS; x <= tx + TERRAIN_DRAW_RADIUS; x += ZONE_SIZE) {
            drawZone(glm::ivec2(x, z), cmdBuffer, descriptorSet); 
        }
    }
}

