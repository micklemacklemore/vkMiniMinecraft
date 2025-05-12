#include "chunk.h"
#include "chunk_constants.h"
#include "vulkan_resources.h"
#include "types.h" 

Chunk::Chunk(int x, int z) : m_blocks(), minX(x), minZ(z), m_neighbors{ {XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr} }
{
    std::fill_n(m_blocks.begin(), 65536, EMPTY);
}

// Does bounds checking with at()
BlockType Chunk::getBlockAt(unsigned int x, unsigned int y, unsigned int z) const {
    return m_blocks.at(x + 16 * y + 16 * 256 * z);
}

// Exists to get rid of compiler warnings about int -> unsigned int implicit conversion
BlockType Chunk::getBlockAt(int x, int y, int z) const {
    return getBlockAt(static_cast<unsigned int>(x), static_cast<unsigned int>(y), static_cast<unsigned int>(z));
}

// Does bounds checking with at()
void Chunk::setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t) {
    m_blocks.at(x + 16 * y + 16 * 256 * z) = t;
}


const static std::unordered_map<Direction, Direction, EnumHash> oppositeDirection{
    {XPOS, XNEG},
    {XNEG, XPOS},
    {YPOS, YNEG},
    {YNEG, YPOS},
    {ZPOS, ZNEG},
    {ZNEG, ZPOS}
};

void Chunk::linkNeighbor(uPtr<Chunk>& neighbor, Direction dir) {
    if (neighbor != nullptr) {
        this->m_neighbors[dir] = neighbor.get();
        neighbor->m_neighbors[oppositeDirection.at(dir)] = this;
    }
}

void createFaceIndices(std::vector<uint32_t>& idxData, const std::array<uint32_t, ChunkConstants::VERT_COUNT>& faceIndices) {
    // 0: UR, 1: LR, 2: LL, 3: UL
    // First Triangle: 0, 3, 1
    idxData.push_back(faceIndices.at(0));
    idxData.push_back(faceIndices.at(3));
    idxData.push_back(faceIndices.at(1));

    // Second Triangle: 1, 3, 2
    idxData.push_back(faceIndices.at(1));
    idxData.push_back(faceIndices.at(3));
    idxData.push_back(faceIndices.at(2));
}

void Chunk::createVertexData(VkDevice device, VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface, VkCommandPool commandPool, VkQueue queue) {
    // check every block to see if it's NOT empty
    // check the neighbours of each non-empty block to see if they ARE empty
    // if a nebour is empty, add VBO data for a face in that direction
        // vertex pos, vertex col, v normal, idx
    std::vector<Vertex> vertexData;
    std::vector<uint32_t> idxData;
    int idxCounter = 0;

    // change this so that it vertices are drawn relative to worldspace (using minX / minZ)
    // zyx because it's more cache efficient
    for (int z = 0; z < 16; z++) {
        for (int y = 0; y < 256; y++) {
            for (int x = 0; x < 16; x++) {
                BlockType current = this->getBlockAt(x, y, z);
                if (current != EMPTY) {
                    for (const ChunkConstants::BlockFace& n : ChunkConstants::neighbouringFaces) {
                        glm::ivec3 offset = glm::ivec3(x, y, z) + n.direction;

                        BlockType neighbour;

                        // TODO: ideally we access neighbouring chunks here
                        if (offset.x < 0 || offset.x > 15 ||
                            offset.y < 0 || offset.y > 255 ||
                            offset.z < 0 || offset.z > 15) {
                            neighbour = EMPTY;
                        }
                        else {
                            neighbour = this->getBlockAt(offset.x, offset.y, offset.z);
                        }

                        if (neighbour == EMPTY) {
                            std::array<uint32_t, ChunkConstants::VERT_COUNT> faceIndices;
                            for (size_t i = 0; i < n.pos.size(); i++) {
                                Vertex vtx; 
                                vtx.pos = glm::vec3(minX + x, y, minZ + z) + glm::vec3(n.pos[i]);
                                vtx.nor = n.nor; 
                                vtx.color = ChunkConstants::blocktype_to_color.at(current);
                                faceIndices.at(i) = idxCounter++;
                                vertexData.push_back(vtx); 
                            }
                            // add index data for this face
                            createFaceIndices(idxData, faceIndices);
                        }
                    }
                }
            }
        }
    }

    vertexSize = vertexData.size(); 
    numIndices = idxData.size();

    createVkBuffer(device, physicalDevice, surface, commandPool, queue, vertexData, idxData);
}

void Chunk::createVkBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface, VkCommandPool commandPool, VkQueue queue, const std::vector<Vertex>& vertex,
    const std::vector<uint32_t>& idx)
{
    // VkDeviceSize bufferSize = sizeof(constants::vertices[0]) * constants::vertices.size();
    bufferSize = (sizeof(Vertex) * vertex.size()) + (sizeof(uint32_t) * idx.size()); 

    // create a staging buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(device, physicalDevice, surface, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    // copy to staging
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertex.data(), vertex.size() * sizeof(Vertex));
    memcpy(static_cast<char*>(data) + (sizeof(Vertex) * vertex.size()),
        idx.data(),
        idx.size() * sizeof(uint32_t));
    vkUnmapMemory(device, stagingBufferMemory);

    // create device bufferand copy to buffer
    createBuffer(device, physicalDevice, surface, bufferSize, 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VertexBuffer, VertexBufferMemory);
    copyBuffer(device, commandPool, queue, stagingBuffer, VertexBuffer, bufferSize);

    // destroy staging buffer
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

