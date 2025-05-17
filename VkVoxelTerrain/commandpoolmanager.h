#include "globals.h"
#include "vulkan_setup.h"
#include <mutex>
#include <thread>
#include <array>
#include <utility>

// CommandPoolManager manages command pools per thread
class CommandPoolManager {
public:
    CommandPoolManager() : 
        mutex_{}, threadPools{}, device{VK_NULL_HANDLE}, queueFamilyIndex{0}
    {
    }

    void init(VkDevice device, uint32_t index)
    {
        this->device = device;
        this->queueFamilyIndex = index;

        for (int i = 0; i < 16; ++i) {
            // Create a new command pool
            VkCommandPool commandPool;
            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.queueFamilyIndex = queueFamilyIndex;
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

            if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create command pool");
            }

            // Emplace pair into vector
            threadPools[i].first = commandPool; 
        }
    }

    std::pair<VkCommandPool, std::mutex*> getCommandPool() {
        for (auto& [pool, mutex] : threadPools) {
            if (mutex.try_lock()) {
                // Caller is responsible for unlocking it later
                return std::make_pair(pool, &mutex);
            }
        }
        
        return { VK_NULL_HANDLE, nullptr }; // Nothing available
    }

    void cleanup() {
        for (auto& [pool, _] : threadPools) {
            vkDestroyCommandPool(device, pool, nullptr);
        }
    }

    void setQueueFamilyIndex(uint32_t index) {
        queueFamilyIndex = index;
    }

private:
    std::mutex mutex_;
    std::array<std::pair<VkCommandPool, std::mutex>, 16> threadPools;
    VkDevice device; 
    uint32_t queueFamilyIndex;
};