#include "globals.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const int MAX_FRAMES_IN_FLIGHT = 2;

float MOUSE_X = 0.f;
float MOUSE_Y = 0.f;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
    "VK_LAYER_LUNARG_monitor"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif
