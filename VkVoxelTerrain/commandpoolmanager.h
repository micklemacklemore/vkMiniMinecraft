#include <vulkan/vulkan.h>
#include <mutex>
#include <unordered_map>
#include <thread>
#include <vector>

// CommandPoolManager manages command pools per thread
class CommandPoolManager {
public:
    CommandPoolManager(VkDevice device, uint32_t qFamilyIndex) : 
        mutex_{}, threadPools{}, device{device}, queueFamilyIndex{qFamilyIndex}
    {
    }

    VkCommandPool getCommandPool() {
        std::lock_guard<std::mutex> lock(mutex_);

        std::thread::id threadId = std::this_thread::get_id();

        auto it = threadPools.find(threadId);
        if (it != threadPools.end()) {
            return it->second;
        }

        // Create a new command pool
        VkCommandPool commandPool;
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndex;  // Set appropriately
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command pool");
        }

        threadPools[threadId] = commandPool;
        return commandPool;
    }

    void cleanup() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& [_, pool] : threadPools) {
            vkDestroyCommandPool(device, pool, nullptr);
        }
        threadPools.clear();
    }

    void setQueueFamilyIndex(uint32_t index) {
        queueFamilyIndex = index;
    }

private:
    std::mutex mutex_;
    std::unordered_map<std::thread::id, VkCommandPool> threadPools;
    VkDevice device; 
    uint32_t queueFamilyIndex;
};