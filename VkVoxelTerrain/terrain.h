#pragma once
#include "globals.h"
#include "smartpointerhelp.h"
#include "glm_includes.h"
#include "chunk.h"
#include "threadpool.h"
#include "commandpoolmanager.h"

#include <array>
#include <unordered_map>
#include <unordered_set>

// Helper functions to convert (x, z) to and from hash map key
int64_t toKey(int x, int z);
glm::ivec2 toCoords(int64_t k);

class Renderer; 

// The container class for all of the Chunks in the game.
// Ultimately, while Terrain will always store all Chunks,
// not all Chunks will be drawn at any given time as the world
// expands.
class Terrain {
    friend Renderer; 
private:
    // Stores every Chunk according to the location of its lower-left corner
    // in world space.
    // We combine the X and Z coordinates of the Chunk's corner into one 64-bit int
    // so that we can use them as a key for the map, as objects like std::pairs or
    // glm::ivec2s are not hashable by default, so they cannot be used as keys.
    Renderer* context; 
    std::unordered_map<int64_t, uPtr<Chunk>> m_chunks;
    std::mutex m_chunks_mutex; 

    // We will designate every 64 x 64 area of the world's x-z plane
    // as one "terrain generation zone". Every time the player moves
    // near a portion of the world that has not yet been generated
    // (i.e. its lower-left coordinates are not in this set), a new
    // 4 x 4 collection of Chunks is created to represent that area
    // of the world.
    // The world that exists when the base code is run consists of exactly
    // one 64 x 64 area with its lower-left corner at (0, 0).
    // When milestone 1 has been implemented, the Player can move around the
    // world to add more "terrain generation zone" IDs to this set.
    // While only the 3 x 3 collection of terrain generation zones
    // surrounding the Player should be rendered, the Chunks
    // in the Terrain will never be deleted until the program is terminated.
    std::unordered_set<int64_t> m_generatedTerrain;
    VkPipeline pipelineChunks;
    ThreadPool threadPool; 

    std::vector<Chunk*> pendingChunks; 
    std::mutex pendingChunksMutex; 

    std::vector<Chunk*> drawableChunks; 
    std::mutex drawableChunksMutex; 

    CommandPoolManager transferCmdPoolManager;
public:
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline* currentPipeline;

    Terrain(Renderer* vulkanContext);
    ~Terrain();

    void buildPipelines();
    void destroyResources(); 

    // Instantiates a new Chunk and stores it in
    // our chunk map at the given coordinates.
    // Returns a pointer to the created Chunk.
    Chunk* instantiateChunkAt(int x, int z);
    void drawZone(glm::ivec2 zone, VkCommandBuffer cmdBuffer, VkDescriptorSet descriptorSet);
    // Do these world-space coordinates lie within
    // a Chunk that exists?
    bool hasChunkAt(int x, int z);
    // Assuming a Chunk exists at these coords,
    // return a mutable reference to it
    uPtr<Chunk>& getChunkAt(int x, int z);
    // Given a world-space coordinate (which may have negative
    // values) return the block stored at that point in space.
    BlockType getBlockAt(int x, int y, int z);
    // Given a world-space coordinate (which may have negative
    // values) set the block at that point in space to the
    // given type.
    void setBlockAt(int x, int y, int z, BlockType t);

    void tryExpansion(const glm::vec3& pos); 

    BlockType createBlock(int x, int y, int z); 

    void threadCreateBlockData(glm::vec2 terrainCoord); 
    void threadCreateBufferData(Chunk* chunk); 

    // Draws every Chunk that falls within the bounding box
    // described by the min and max coords, using the provided
    // ShaderProgram
    void draw(const glm::vec3& position, VkCommandBuffer cmdBuffer, VkDescriptorSet descriptorSet);
};