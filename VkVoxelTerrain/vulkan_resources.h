#pragma once

#include "globals.h"
#include "types.h"
#include "vulkan_setup.h"

#include <iostream>     // std::cerr
#include <stdexcept>    // std::runtime_error
#include <vector>       // std::vector

// Finds suitable memory type from the GPU based on filter and desired properties.
uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

// Finds a supported format given a list of candidates (order from high to low priority), tiling and features requirements. 
VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates,
    VkImageTiling tiling, VkFormatFeatureFlags features);

// Finds a suitable depth format. 
VkFormat findDepthFormat(VkPhysicalDevice physicalDevice); 

// Check if format has stencil component. 
bool hasStencilComponent(VkFormat format); 

// Helper functions for recording + submitting single-use commands. Command pool should be transient. 
VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
void endSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkCommandBuffer commandBuffer); 

// Creates a Vulkan buffer and allocates device memory for it.
void createBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkDeviceSize size,
    VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

// Copies data from one buffer to another using a temporary command buffer. Command pool should be transient. 
void copyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue queue,
    VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

// Creates image object and associated memory bound to it.
void createImage(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
    uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
    VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image,
    VkDeviceMemory& imageMemory);

// Copies buffer data into a Vulkan image.
void copyBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue queue,
    VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

// Transitions the layout of a Vulkan image (e.g., undefined -> shader read).
void transitionImageLayout(VkDevice device, VkCommandPool commandPool, VkQueue queue,
    VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

// creates a 2D imageview
VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

// ====== Implementations ======

