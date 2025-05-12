#include "globals.h"
#include "camera_fps.h"
#include "types.h"
#include "vulkan_setup.h"
#include "vulkan_swapchain.h"
#include "vulkan_resources.h" 
#include "terrain.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>
#include <array>
#include <algorithm>

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

//namespace constants
//{
//    const std::vector<Vertex> vertices = {
//        // Front face
//        {{-0.5f, -0.5f,  0.5f}, {1, 0, 0}, {0.0f, 0.0f}}, // 0
//        {{ 0.5f, -0.5f,  0.5f}, {0, 1, 0}, {1.0f, 0.0f}}, // 1
//        {{ 0.5f,  0.5f,  0.5f}, {0, 0, 1}, {1.0f, 1.0f}}, // 2
//        {{-0.5f,  0.5f,  0.5f}, {1, 1, 1}, {0.0f, 1.0f}}, // 3
//
//        // Back face
//        {{ 0.5f, -0.5f, -0.5f}, {0, 1, 1}, {0.0f, 0.0f}}, // 4
//        {{-0.5f, -0.5f, -0.5f}, {1, 1, 0}, {1.0f, 0.0f}}, // 5
//        {{-0.5f,  0.5f, -0.5f}, {0.5, 0.5, 0.5}, {1.0f, 1.0f}}, // 6
//        {{ 0.5f,  0.5f, -0.5f}, {1, 0, 1}, {0.0f, 1.0f}}, // 7
//
//        // Left face
//        {{-0.5f, -0.5f, -0.5f}, {1, 1, 0}, {0.0f, 0.0f}}, // 8
//        {{-0.5f, -0.5f,  0.5f}, {1, 0, 0}, {1.0f, 0.0f}}, // 9
//        {{-0.5f,  0.5f,  0.5f}, {1, 1, 1}, {1.0f, 1.0f}}, // 10
//        {{-0.5f,  0.5f, -0.5f}, {0.5, 0.5, 0.5}, {0.0f, 1.0f}}, // 11
//
//        // Right face
//        {{ 0.5f, -0.5f,  0.5f}, {0, 1, 0}, {0.0f, 0.0f}}, // 12
//        {{ 0.5f, -0.5f, -0.5f}, {0, 1, 1}, {1.0f, 0.0f}}, // 13
//        {{ 0.5f,  0.5f, -0.5f}, {1, 0, 1}, {1.0f, 1.0f}}, // 14
//        {{ 0.5f,  0.5f,  0.5f}, {0, 0, 1}, {0.0f, 1.0f}}, // 15
//
//        // Top face
//        {{-0.5f,  0.5f,  0.5f}, {1, 1, 1}, {0.0f, 0.0f}}, // 16
//        {{ 0.5f,  0.5f,  0.5f}, {0, 0, 1}, {1.0f, 0.0f}}, // 17
//        {{ 0.5f,  0.5f, -0.5f}, {1, 0, 1}, {1.0f, 1.0f}}, // 18
//        {{-0.5f,  0.5f, -0.5f}, {0.5, 0.5, 0.5}, {0.0f, 1.0f}}, // 19
//
//        // Bottom face
//        {{-0.5f, -0.5f, -0.5f}, {1, 1, 0}, {0.0f, 0.0f}}, // 20
//        {{ 0.5f, -0.5f, -0.5f}, {0, 1, 1}, {1.0f, 0.0f}}, // 21
//        {{ 0.5f, -0.5f,  0.5f}, {0, 1, 0}, {1.0f, 1.0f}}, // 22
//        {{-0.5f, -0.5f,  0.5f}, {1, 0, 0}, {0.0f, 1.0f}}, // 23
//    };
//
//    const std::vector<uint16_t> indices = {
//        // Front face
//        0, 1, 2, 2, 3, 0,
//        // Back face
//        4, 5, 6, 6, 7, 4,
//        // Left face
//        8, 9,10,10,11, 8,
//        // Right face
//        12,13,14,14,15,12,
//        // Top face
//        16,17,18,18,19,16,
//        // Bottom face
//        20,21,22,22,23,20
//    };
//}


class Renderer {
public:
    void run() {
        initWindow();
        initVulkan();

        terrain.CreateTestScene(device, physicalDevice, surface, commandPoolTransfer, queueTransfer); 

        mainLoop();
        cleanup();
    }
private:
    float rotate = 0.f; 
    float zoom = 0.f; 

    GLFWwindow* window;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue queueGraphics;
    VkQueue queuePresent;
    VkQueue queueTransfer; 

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    VkRenderPass renderPass;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    VkCommandPool commandPoolGraphics;
    std::vector<VkCommandBuffer> commandBuffersGraphics;

    VkCommandPool commandPoolTransfer; 

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;

    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    bool framebufferResized = false;

    CameraFPS camera{ WIDTH, HEIGHT, glm::vec3(64., 135., 64.) };
    Terrain terrain; 

    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Voxel Terrain", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window, mouseCallback);
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    void initVulkan() {
        // Initialize Vulkan instance, devices, surfaces and queues
        createInstance(instance);
        setupDebugMessenger(instance, debugMessenger);
        createSurface(instance, window, surface);
        pickPhysicalDevice(instance, surface, physicalDevice);
        createLogicalDevice(physicalDevice, surface, device, queueGraphics, queuePresent, queueTransfer);

        // Initialize swapchain
        createSwapChain(
            device,
            physicalDevice,
            surface,
            window,
            swapChain,
            swapChainImages,
            swapChainImageFormat,
            swapChainExtent
        ); 
        createSwapChainImageViews(
            device,
            swapChainImageFormat,
            swapChainImageViews,
            swapChainImages
        );

        // Create commmand pools, per-frame command buffers, sync 
        createCommandPools();
        createPerFrameCommandBuffers();
        createSyncObjects();

        // Create depth buffer
        createDepthResources();

        // Create renderpass and framebuffer
        createRenderPass();
        createFramebuffers();

        // Main graphics pipeline
        terrain.buildPipelines(device, renderPass); 

        // Inputs to shaders
        createTextureImage(); 
        createTextureImageView(); 
        createTextureSampler(); 
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets(); 
    }

    void mainLoop() {
        float deltaTime = 0.0f; // Time between current frame and last frame
        float lastFrame = 0.0f; // Time of last frame

        while (!glfwWindowShouldClose(window)) {
            float currentFrame = glfwGetTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            processInput(window, deltaTime); 

            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(device);
    }

    void processInput(GLFWwindow* window, float delta)
    {
        static Input input;
        input.reset();

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            input.wPressed = true; 
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            input.aPressed = true; 
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            input.sPressed = true; 
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            input.dPressed = true; 
        }
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            input.ePressed = true;
        }
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            input.qPressed = true;
        }

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE); 
        }

        input.mouseX = (int)MOUSE_X; 
        input.mouseY = (int)MOUSE_Y; 

        camera.processInput(input, delta); 

        MOUSE_X = 0.f; 
        MOUSE_Y = 0.f; 
    }

    static void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
        static double lastX = 400, lastY = 300;
        static bool firstMouse = true;

        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        MOUSE_X = xpos - lastX;
        MOUSE_Y = lastY - ypos;  // Reversed since y-coordinates range top to bottom

        lastX = xpos;
        lastY = ypos;
    }

    void cleanupSwapChain() {
        vkDestroyImageView(device, depthImageView, nullptr);
        vkDestroyImage(device, depthImage, nullptr);
        vkFreeMemory(device, depthImageMemory, nullptr);

        for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(device, imageView, nullptr);
        }

        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }

    void cleanup() {
        cleanupSwapChain();

        vkDestroySampler(device, textureSampler, nullptr);
        vkDestroyImageView(device, textureImageView, nullptr);

        vkDestroyImage(device, textureImage, nullptr);
        vkFreeMemory(device, textureImageMemory, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroyBuffer(device, uniformBuffers[i], nullptr);
            vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
        }

        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        
        terrain.destroyVkResources(device); 

        vkDestroyRenderPass(device, renderPass, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(device, commandPoolGraphics, nullptr);
        vkDestroyCommandPool(device, commandPoolTransfer, nullptr); 

        vkDestroyDevice(device, nullptr);

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }

    void recreateSwapChain() {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(device);

        cleanupSwapChain();

        createSwapChain(
            device,
            physicalDevice,
            surface,
            window, 
            swapChain,
            swapChainImages,
            swapChainImageFormat,
            swapChainExtent
        );
        createSwapChainImageViews(
            device,
            swapChainImageFormat,
            swapChainImageViews,
            swapChainImages
        );
        createDepthResources();
        createFramebuffers();
    }

    void createRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = findDepthFormat(physicalDevice);
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void createDescriptorSets() {
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, terrain.descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = textureImageView;
            imageInfo.sampler = textureSampler;

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void createTextureImage() {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load("textures/minecraft_textures_all.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }

        // create a staging buffer
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        createBuffer(device, physicalDevice, surface, 
            imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            stagingBuffer, stagingBufferMemory);

        // copy image into staging buffer
        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(device, stagingBufferMemory);

        stbi_image_free(pixels);

        // create image object
        createImage(device, physicalDevice, surface, 
            texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, 
            VK_IMAGE_TILING_OPTIMAL, 
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
            textureImage, 
            textureImageMemory
        );

        // copy staging buffer to image
        transitionImageLayout(device, commandPoolTransfer, queueTransfer, 
            textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, 
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        copyBufferToImage(device, commandPoolTransfer, queueTransfer, stagingBuffer, textureImage, 
            static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

        // transition to shader read
        transitionImageLayout(device, commandPoolGraphics, queueGraphics,
            textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    void createTextureImageView() {
        textureImageView = createImageView(device, textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    void createDepthResources() {
        VkFormat depthFormat = findDepthFormat(physicalDevice);
        createImage(device, physicalDevice, surface,
            swapChainExtent.width, swapChainExtent.height, 
            depthFormat, VK_IMAGE_TILING_OPTIMAL, 
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
        depthImageView = createImageView(device, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    void createTextureSampler() {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(physicalDevice, &properties);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_NEAREST;
        samplerInfo.minFilter = VK_FILTER_NEAREST;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }

    void createUniformBuffers() {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            createBuffer(device, physicalDevice, surface, bufferSize,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                uniformBuffers[i], 
                uniformBuffersMemory[i]
            );

            vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
        }
    }

    void createFramebuffers() {
        swapChainFramebuffers.resize(swapChainImageViews.size());

        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            std::array<VkImageView, 2> attachments = {
                swapChainImageViews[i],
                depthImageView
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void createCommandPools() {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice, surface);

        {
            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

            if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPoolGraphics) != VK_SUCCESS) {
                throw std::runtime_error("failed to create command pool!");
            }
        }

        {
            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
            poolInfo.queueFamilyIndex = queueFamilyIndices.transferFamily.value();

            if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPoolTransfer) != VK_SUCCESS) {
                throw std::runtime_error("failed to create command pool!");
            }
        }
    }

    void createPerFrameCommandBuffers() {
        commandBuffersGraphics.resize(MAX_FRAMES_IN_FLIGHT);

        {
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = commandPoolGraphics;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = (uint32_t)commandBuffersGraphics.size();

            if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffersGraphics.data()) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate command buffers!");
            }
        }
    }

    void createDescriptorPool() {
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapChainExtent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.509, 0.784, 0.898, 1.0f} };  // sky blue! :3
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // update viewport / scissor
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapChainExtent.width;
        viewport.height = (float)swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        // draw

        // VkBuffer vertexBuffers[] = { vertexBuffer };
        // VkDeviceSize offsets[] = { 0 };
        // vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        // vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
        // vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, terrain.pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);
        // terrain.draw(0, 64, 0, 64, commandBuffer, terrain.pipelineLayout);

        terrain.draw(0, 64, 0, 64, commandBuffer, descriptorSets[currentFrame]);

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void updateUniformBuffer(uint32_t currentImage) {
        UniformBufferObject ubo{};
        ubo.model = glm::mat4(1.0f); 
        
        ubo.viewproj = camera.getViewProjectionMatrix(); 
        memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }

    void createSyncObjects() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    void drawFrame() {
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        vkResetCommandBuffer(commandBuffersGraphics[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        recordCommandBuffer(commandBuffersGraphics[currentFrame], imageIndex);

        updateUniformBuffer(currentFrame);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffersGraphics[currentFrame];

        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(queueGraphics, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(queuePresent, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
};

int main() {
    Renderer app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}