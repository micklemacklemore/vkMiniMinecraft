#pragma once

#include "globals.h"
#include "types.h"
#include "vulkan_setup.h"
#include "vulkan_resources.h"

#include <iostream>     // std::cerr
#include <stdexcept>    // std::runtime_error
#include <vector>       // std::vector
#include <algorithm>    // std::clamp

// === Swapchain Creation ===

void createSwapChain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
    GLFWwindow* window, VkSwapchainKHR& swapChain, std::vector<VkImage>& swapChainImages,
    VkFormat& swapChainImageFormat, VkExtent2D& swapChainExtent); 
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats); 
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes); 
VkExtent2D chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities); 
void createSwapChainImageViews(VkDevice device, VkFormat swapChainImageFormat,
    std::vector<VkImageView>& swapChainImageViews, std::vector<VkImage>& swapChainImages);
