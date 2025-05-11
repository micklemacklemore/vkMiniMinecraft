#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stb_image.h>

#include <cstdint>
#include <vector>

extern const uint32_t WIDTH;
extern const uint32_t HEIGHT;

extern const int MAX_FRAMES_IN_FLIGHT;

extern float MOUSE_X;
extern float MOUSE_Y;

extern const std::vector<const char*> validationLayers;
extern const std::vector<const char*> deviceExtensions;

extern const bool enableValidationLayers;

