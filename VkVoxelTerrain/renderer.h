#ifndef RENDERER_H
#define RENDERER_H

#include "globals.h"
#include "glm_includes.h"
#include "terrain.h"
#include "camera_fps.h"

class Renderer {
    friend Terrain;
public:
    Renderer(); 
    void run();

private:
    void initWindow();
    void initVulkan();
    void initImGui();
    void mainLoop();
    void processInput(GLFWwindow* window, float delta);
    void createGUIOverlay();
    void cleanup();
    void cleanupSwapChain();
    void recreateSwapChain();
    void createRenderPass();
    void createDescriptorSets();
    void createTextureImage();
    void createTextureImageView();
    void createDepthResources();
    void createTextureSampler();
    void createUniformBuffers();
    void createFramebuffers();
    void createCommandPools();
    void createPerFrameCommandBuffers();
    void createDescriptorPool();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void updateUniformBuffer(uint32_t currentImage);
    void createSyncObjects();
    void drawFrame();

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);

    float rotate;
    float zoom;

    GLFWwindow* window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VkDevice device;

    VkQueue queueGraphics, queuePresent, queueTransfer;
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

    VkCommandPool commandPoolGraphics, commandPoolTransfer;
    std::vector<VkCommandBuffer> commandBuffersGraphics;

    std::vector<VkSemaphore> imageAvailableSemaphores, renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame;

    uint32_t mipLevels; 
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    bool framebufferResized;
    CameraFPS camera;
    Terrain terrain;
};

#endif // RENDERER_H
