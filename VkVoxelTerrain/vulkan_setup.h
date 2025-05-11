#pragma once

#include "globals.h"
#include "types.h"

#include <iostream>     // std::cerr
#include <fstream>
#include <stdexcept>    // std::runtime_error
#include <vector>       // std::vector
#include <cstring>      // strcmp
#include <set>          // std::set

// === Vulkan Debug and Validation Utilities ===

/// Callback for Vulkan validation layer messages.
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
);

/// Creates a debug messenger for Vulkan validation layers.
VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger
);

/// Destroys the previously created Vulkan debug messenger.
void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator
);

/// Checks whether all requested validation layers are supported.
bool checkValidationLayerSupport();

/// Initializes and sets up the Vulkan debug messenger.
void setupDebugMessenger(VkInstance& instance, VkDebugUtilsMessengerEXT& debugMessenger);

/// Populates a VkDebugUtilsMessengerCreateInfoEXT struct with desired settings.
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);


// === Vulkan Instance and Surface Initialization ===

/// Returns a list of required Vulkan extensions, including debug extensions if enabled.
std::vector<const char*> getRequiredExtensions();

/// Creates a Vulkan instance, optionally enabling validation layers.
void createInstance(VkInstance& instance);

/// Creates a Vulkan surface using GLFW and the given window handle.
void createSurface(VkInstance instance, GLFWwindow* window, VkSurfaceKHR& surface);


// === Physical and Logical Device Setup ===

/// Finds queue families that support graphics, presentation, transfer and compute(TODO) operations.
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

/// Determines whether a given Vulkan physical device meets the required criteria.
bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);

/// Checks if a Vulkan physical device supports all required extensions.
bool checkDeviceExtensionSupport(VkPhysicalDevice device);

/// Queries support details of a physical device's swapchain capabilities.
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

/// Selects a suitable physical GPU from the system that supports required features.
void pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice& physicalDevice);

/// Creates a Vulkan logical device and retrieves queues for graphics, presentation, transfer,
/// and compute(TODO). 
void createLogicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
    VkDevice& device, VkQueue& queueGraphics,
    VkQueue& queuePresent, VkQueue& queueTransfer);

VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code); 

std::vector<char> readFile(const std::string& filename);

