#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

#include <iostream>
#include <vector>
#include <array>

#include "vulkantools.h"
#include "vulkandebug.h"

#include "vulkanswapchain.hpp"

#define VERTEX_BUFFER_BIND_ID 0

#define deg_to_rad(deg) deg * float(M_PI / 180)

VkResult createInstance(bool enableValidation, VkInstance& instance);
VkResult createDevice(const VkPhysicalDevice& physicalDevice, VkDeviceQueueCreateInfo requestedQueues, bool enableValidation, VkDevice& device);

// Creates a new (graphics) command pool object storing command buffers
void createCommandPool();
// Setup default depth and stencil views
void setupDepthStencil();
// Create framebuffers for all requested swap chain images
void setupFrameBuffer();
// Setup a default render pass
void setupRenderPass();

// Create swap chain images
void setupSwapChain();

// Check if command buffers are valid (!= VK_NULL_HANDLE)
bool checkCommandBuffers();
// Create command buffers for drawing commands
void createCommandBuffers();
// Destroy all command buffers and set their handles to VK_NULL_HANDLE
// May be necessary during runtime if options are toggled 
void destroyCommandBuffers();
// Create command buffer for setup commands
void createSetupCommandBuffer();
// Finalize setup command bufferm submit it to the queue and remove it
void flushSetupCommandBuffer();

// Create a cache pool for rendering pipelines
void createPipelineCache();

// Get memory type for a given memory allocation (flags and bits)
VkBool32 getMemoryType(uint32_t typeBits, VkFlags properties, uint32_t *typeIndex);

// Load a SPIR-V shader
VkPipelineShaderStageCreateInfo loadShader(const char* fileName, VkShaderStageFlagBits stage);
// Load a GLSL shader
// NOTE : This may not work with any IHV and requires some magic
VkPipelineShaderStageCreateInfo loadShaderGLSL(const char* fileName, VkShaderStageFlagBits stage);

// Create a buffer, fill it with data and bind buffer memory
// Can be used for e.g. vertex or index buffer based on mesh data
VkBool32 createBuffer(
    VkBufferUsageFlags usage,
    VkDeviceSize size,
    void *data,
    VkBuffer *buffer,
    VkDeviceMemory *memory);
// Overload that assigns buffer info to descriptor
VkBool32 createBuffer(
    VkBufferUsageFlags usage,
    VkDeviceSize size,
    void *data,
    VkBuffer *buffer,
    VkDeviceMemory *memory,
    VkDescriptorBufferInfo *descriptor);



void createSetupCommandBuffer();
void prepareVertices();
void prepareUniformBuffers();
void setupDescriptorSetLayout();
void preparePipelines();
void setupDescriptorPool();
void setupDescriptorSet();
void buildCommandBuffers();
void updateUniformBuffers();
void render();



auto enableValidation = false;

// Vulkan instance, stores all per-application states
VkInstance instance;
// Physical device (GPU) that Vulkan will ise
VkPhysicalDevice physicalDevice;
// Stores all available memory (type) properties for the physical device
VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
// Logical device, application's view of the physical device (GPU)
VkDevice device;
// Handle to the device graphics queue that command buffers are submitted to
VkQueue queue;
// Color buffer format
VkFormat colorformat = VK_FORMAT_B8G8R8A8_UNORM;
// Depth buffer format
// Depth format is selected during Vulkan initialization
VkFormat depthFormat;
// Command buffer pool
VkCommandPool cmdPool;
// Command buffer used for setup
VkCommandBuffer setupCmdBuffer = VK_NULL_HANDLE;
// Command buffer for submitting a post present barrier
VkCommandBuffer postPresentCmdBuffer = VK_NULL_HANDLE;
// Command buffers used for rendering
std::vector<VkCommandBuffer> drawCmdBuffers;
// Global render pass for frame buffer writes
VkRenderPass renderPass;
// List of available frame buffers (same as number of swap chain images)
std::vector<VkFramebuffer>frameBuffers;
// Active frame buffer index
uint32_t currentBuffer = 0;
// Descriptor set pool
VkDescriptorPool descriptorPool;
// List of shader modules created (stored for cleanup)
std::vector<VkShaderModule> shaderModules;
// Pipeline cache object
VkPipelineCache pipelineCache;
// Wraps the swap chain to present images (framebuffers) to the windowing system
VulkanSwapChain swapChain;


uint32_t width = 1280;
uint32_t height = 720;

VkClearColorValue defaultClearColor = { { 0.025f, 0.025f, 0.025f, 1.0f } };

float zoom = 0;

// Defines a frame rate independent timer value clamped from -1.0...1.0
// For use in animations, rotations, etc.
float timer = 0.0f;
// Multiplier for speeding up (or slowing down) the global timer
float timerSpeed = 0.25f;

bool paused = false;

// Use to adjust mouse rotation speed
float rotationSpeed = 1.0f;
// Use to adjust mouse zoom speed
float zoomSpeed = 1.0f;

glm::vec3 rotation = glm::vec3();
glm::vec2 mousePos;

std::string title = "Vulkan Example";
std::string name = "vulkanExample";

struct
{
    VkImage image;
    VkDeviceMemory mem;
    VkImageView view;
} depthStencil;


struct {
    VkBuffer buf;
    VkDeviceMemory mem;
    VkPipelineVertexInputStateCreateInfo vi;
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
} vertices;

struct {
    int count;
    VkBuffer buf;
    VkDeviceMemory mem;
} indices;

struct {
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDescriptorBufferInfo descriptor;
}  uniformDataVS;

struct {
    glm::mat4 projectionMatrix;
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
} uboVS;

struct {
    VkPipeline solid;
} pipelines;

VkPipelineLayout pipelineLayout;
VkDescriptorSet descriptorSet;
VkDescriptorSetLayout descriptorSetLayout;

int main_old(void)
{
    if (!glfwInit())
        return -1;

    if (!glfwVulkanSupported())
    {
        return -1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    int count;
    const char** extensions = glfwGetRequiredInstanceExtensions(&count);
    if (!extensions) {
        return -1;
    }

    for (auto i = 0; i < count; ++i) {
        std::cout << extensions[i] << std::endl;
    }

    VkResult err;

    // Vulkan instance
    err = createInstance(enableValidation, instance);
    if (err)
    {
        vkTools::exitFatal("Could not create Vulkan instance : \n" + vkTools::errorString(err), "Fatal error");
    }

    // Physical device
    // Note : This example will always use the first physical device reported
    uint32_t gpuCount;
    err = vkEnumeratePhysicalDevices(instance, &gpuCount, &physicalDevice);
    if (err)
    {
        vkTools::exitFatal("Could not enumerate phyiscal devices : \n" + vkTools::errorString(err), "Fatal error");
    }

    // Find a queue that supports graphics operations
    uint32_t graphicsQueueIndex = 0;
    uint32_t queueCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, NULL);
    assert(queueCount >= 1);

    std::vector<VkQueueFamilyProperties> queueProps;
    queueProps.resize(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queueProps.data());

    for (graphicsQueueIndex = 0; graphicsQueueIndex < queueCount; graphicsQueueIndex++)
    {
        if (queueProps[graphicsQueueIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            break;
    }
    assert(graphicsQueueIndex < queueCount);

    // Vulkan device
    std::array<float, 1> queuePriorities = { 0.0f };
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = graphicsQueueIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = queuePriorities.data();

    err = createDevice(physicalDevice, queueCreateInfo, enableValidation, device);
    assert(!err);

    // Gather physical device memory properties
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);

    // Get the graphics queue
    vkGetDeviceQueue(device, graphicsQueueIndex, 0, &queue);

    // Find supported depth format
    // We prefer 24 bits of depth and 8 bits of stencil, but that may not be supported by all implementations
    std::vector<VkFormat> depthFormats = { VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM };
    bool depthFormatFound = false;
    for (auto& format : depthFormats)
    {
        VkFormatProperties formatProps;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
        // Format must support depth stencil attachment for optimal tiling
        if (formatProps.optimalTilingFeatures && VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            depthFormat = format;
            depthFormatFound = true;
            break;
        }
    }

    assert(depthFormatFound);

    GLFWwindow* window;

    window = glfwCreateWindow(width, height, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    swapChain.init(instance, physicalDevice, device);
    swapChain.initSwapChain(window);
    
    if (enableValidation)
    {
        vkDebug::setupDebugging(instance, VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT, NULL);
    }

    createCommandPool();
    createSetupCommandBuffer();
    setupSwapChain();
    createCommandBuffers();
    setupDepthStencil();
    setupRenderPass();
    createPipelineCache();
    setupFrameBuffer();
    flushSetupCommandBuffer();
    // Recreate setup command buffer for derived class
    createSetupCommandBuffer();
    prepareVertices();
    prepareUniformBuffers();
    setupDescriptorSetLayout();
    preparePipelines();
    setupDescriptorPool();
    setupDescriptorSet();
    buildCommandBuffers();


    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        render();

        /* Swap front and back buffers */
        //glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    swapChain.cleanup();

    glfwTerminate();
    return 0;
}



VkResult createInstance(bool enableValidation, VkInstance& instance)
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = name.c_str();
    appInfo.pEngineName = name.c_str();
    // Temporary workaround for drivers not supporting SDK 1.0.3 upon launch
    // todo : Use VK_API_VERSION 
    appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 2);

    std::vector<const char*> enabledExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };

#ifdef _WIN32
    enabledExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#else
    // todo : linux/android
    enabledExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif

    // todo : check if all extensions are present 

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = NULL;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    if (enabledExtensions.size() > 0)
    {
        if (enableValidation)
        {
            enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        }
        instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
        instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
    }
    if (enableValidation)
    {
        instanceCreateInfo.enabledLayerCount = vkDebug::validationLayerCount; // todo : change validation layer names!
        instanceCreateInfo.ppEnabledLayerNames = vkDebug::validationLayerNames;
    }
    return vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
}

VkResult createDevice(const VkPhysicalDevice& physicalDevice, VkDeviceQueueCreateInfo requestedQueues, bool enableValidation, VkDevice& device)
{
    std::vector<const char*> enabledExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = NULL;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &requestedQueues;
    deviceCreateInfo.pEnabledFeatures = NULL;

    if (enabledExtensions.size() > 0)
    {
        deviceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
        deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
    }
    if (enableValidation)
    {
        deviceCreateInfo.enabledLayerCount = vkDebug::validationLayerCount; // todo : validation layer names
        deviceCreateInfo.ppEnabledLayerNames = vkDebug::validationLayerNames;
    }

    return vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
}


bool checkCommandBuffers(const std::vector<VkCommandBuffer>& drawCmdBuffers)
{
    for (auto& cmdBuffer : drawCmdBuffers)
    {
        if (cmdBuffer == VK_NULL_HANDLE)
        {
            return false;
        }
    }
    return true;
}

void createCommandBuffers()
{
    // Create one command buffer per frame buffer 
    // in the swap chain
    // Command buffers store a reference to the 
    // frame buffer inside their render pass info
    // so for static usage withouth having to rebuild 
    // them each frame, we use one per frame buffer

    drawCmdBuffers.resize(swapChain.imageCount);

    VkCommandBufferAllocateInfo cmdBufAllocateInfo =
        vkTools::initializers::commandBufferAllocateInfo(
            cmdPool,
            VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            (uint32_t)drawCmdBuffers.size());

    VkResult vkRes = vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, drawCmdBuffers.data());
    assert(!vkRes);

    // Create one command buffer for submitting the
    // post present image memory barrier
    cmdBufAllocateInfo.commandBufferCount = 1;

    vkRes = vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, &postPresentCmdBuffer);
    assert(!vkRes);
}

void destroyCommandBuffers()
{
    vkFreeCommandBuffers(device, cmdPool, (uint32_t)drawCmdBuffers.size(), drawCmdBuffers.data());
    vkFreeCommandBuffers(device, cmdPool, 1, &postPresentCmdBuffer);
}

void createSetupCommandBuffer()
{
    if (setupCmdBuffer != VK_NULL_HANDLE)
    {
        vkFreeCommandBuffers(device, cmdPool, 1, &setupCmdBuffer);
        setupCmdBuffer = VK_NULL_HANDLE; // todo : check if still necessary
    }

    VkCommandBufferAllocateInfo cmdBufAllocateInfo =
        vkTools::initializers::commandBufferAllocateInfo(
            cmdPool,
            VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            1);

    VkResult vkRes = vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, &setupCmdBuffer);
    assert(!vkRes);

    // todo : Command buffer is also started here, better put somewhere else
    // todo : Check if necessaray at all...
    VkCommandBufferBeginInfo cmdBufInfo = {};
    cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // todo : check null handles, flags?

    vkRes = vkBeginCommandBuffer(setupCmdBuffer, &cmdBufInfo);
    assert(!vkRes);
}

void flushSetupCommandBuffer()
{
    VkResult err;

    if (setupCmdBuffer == VK_NULL_HANDLE)
        return;

    err = vkEndCommandBuffer(setupCmdBuffer);
    assert(!err);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &setupCmdBuffer;

    err = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    assert(!err);

    err = vkQueueWaitIdle(queue);
    assert(!err);

    vkFreeCommandBuffers(device, cmdPool, 1, &setupCmdBuffer);
    setupCmdBuffer = VK_NULL_HANDLE; // todo : check if still necessary
}

void createPipelineCache()
{
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VkResult err = vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &pipelineCache);
    assert(!err);
}

// Build separate command buffers for every framebuffer image
// Unlike in OpenGL all rendering commands are recorded once
// into command buffers that are then resubmitted to the queue
void buildCommandBuffers()
{
    VkCommandBufferBeginInfo cmdBufInfo = {};
    cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufInfo.pNext = NULL;

    VkClearValue clearValues[2];
    clearValues[0].color = defaultClearColor;
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = NULL;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = width;
    renderPassBeginInfo.renderArea.extent.height = height;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;

    VkResult err;

    for (int32_t i = 0; i < drawCmdBuffers.size(); ++i)
    {
        // Set target frame buffer
        renderPassBeginInfo.framebuffer = frameBuffers[i];

        err = vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo);
        assert(!err);

        vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Update dynamic viewport state
        VkViewport viewport = {};
        viewport.height = (float)height;
        viewport.width = (float)width;
        viewport.minDepth = (float) 0.0f;
        viewport.maxDepth = (float) 1.0f;
        vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);

        // Update dynamic scissor state
        VkRect2D scissor = {};
        scissor.extent.width = width;
        scissor.extent.height = height;
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);

        // Bind descriptor sets describing shader binding points
        vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

        // Bind the rendering pipeline (including the shaders)
        vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.solid);

        // Bind triangle vertices
        VkDeviceSize offsets[1] = { 0 };
        vkCmdBindVertexBuffers(drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &vertices.buf, offsets);

        // Bind triangle indices
        vkCmdBindIndexBuffer(drawCmdBuffers[i], indices.buf, 0, VK_INDEX_TYPE_UINT32);

        // Draw indexed triangle
        vkCmdDrawIndexed(drawCmdBuffers[i], indices.count, 1, 0, 0, 1);

        vkCmdEndRenderPass(drawCmdBuffers[i]);

        // Add a present memory barrier to the end of the command buffer
        // This will transform the frame buffer color attachment to a
        // new layout for presenting it to the windowing system integration 
        VkImageMemoryBarrier prePresentBarrier = {};
        prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        prePresentBarrier.pNext = NULL;
        prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        prePresentBarrier.dstAccessMask = 0;
        prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        prePresentBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        prePresentBarrier.image = swapChain.buffers[i].image;

        VkImageMemoryBarrier *pMemoryBarrier = &prePresentBarrier;
        vkCmdPipelineBarrier(
            drawCmdBuffers[i],
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_FLAGS_NONE,
            0, nullptr,
            0, nullptr,
            1, &prePresentBarrier);

        err = vkEndCommandBuffer(drawCmdBuffers[i]);
        assert(!err);
    }
}

void draw()
{
    VkResult err;
    VkSemaphore presentCompleteSemaphore;
    VkSemaphoreCreateInfo presentCompleteSemaphoreCreateInfo = {};
    presentCompleteSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    presentCompleteSemaphoreCreateInfo.pNext = NULL;
    presentCompleteSemaphoreCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    err = vkCreateSemaphore(device, &presentCompleteSemaphoreCreateInfo, nullptr, &presentCompleteSemaphore);
    assert(!err);

    // Get next image in the swap chain (back/front buffer)
    err = swapChain.acquireNextImage(presentCompleteSemaphore, &currentBuffer);
    assert(!err);

    // The submit infor strcuture contains a list of
    // command buffers and semaphores to be submitted to a queue
    // If you want to submit multiple command buffers, pass an array
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &presentCompleteSemaphore;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];

    // Submit to the graphics queue
    err = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    assert(!err);

    // Present the current buffer to the swap chain
    // This will display the image
    err = swapChain.queuePresent(queue, currentBuffer);
    assert(!err);

    vkDestroySemaphore(device, presentCompleteSemaphore, nullptr);

    // Add a post present image memory barrier
    // This will transform the frame buffer color attachment back
    // to it's initial layout after it has been presented to the
    // windowing system
    // See buildCommandBuffers for the pre present barrier that 
    // does the opposite transformation 
    VkImageMemoryBarrier postPresentBarrier = {};
    postPresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    postPresentBarrier.pNext = NULL;
    postPresentBarrier.srcAccessMask = 0;
    postPresentBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    postPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    postPresentBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    postPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    postPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    postPresentBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    postPresentBarrier.image = swapChain.buffers[currentBuffer].image;

    // Use dedicated command buffer from example base class for submitting the post present barrier
    VkCommandBufferBeginInfo cmdBufInfo = {};
    cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    err = vkBeginCommandBuffer(postPresentCmdBuffer, &cmdBufInfo);
    assert(!err);

    // Put post present barrier into command buffer
    vkCmdPipelineBarrier(
        postPresentCmdBuffer,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_FLAGS_NONE,
        0, nullptr,
        0, nullptr,
        1, &postPresentBarrier);

    err = vkEndCommandBuffer(postPresentCmdBuffer);
    assert(!err);

    // Submit to the queue
    submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &postPresentCmdBuffer;

    err = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    assert(!err);

    err = vkQueueWaitIdle(queue);
    assert(!err);
}

// Setups vertex and index buffers for an indexed triangle,
// uploads them to the VRAM and sets binding points and attribute
// descriptions to match locations inside the shaders
void prepareVertices()
{
    struct Vertex {
        float pos[3];
        float col[3];
    };

    // Setup vertices
    std::vector<Vertex> vertexBuffer = {
        { { 1.0f,  1.0f, 0.0f },{ 1.0f, 0.0f, 0.0f } },
        { { -1.0f,  1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } },
        { { 0.0f, -1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } }
    };
    int vertexBufferSize = vertexBuffer.size() * sizeof(Vertex);

    // Setup indices
    std::vector<uint32_t> indexBuffer = { 0, 1, 2 };
    int indexBufferSize = indexBuffer.size() * sizeof(uint32_t);

    VkMemoryAllocateInfo memAlloc = {};
    memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAlloc.pNext = NULL;
    memAlloc.allocationSize = 0;
    memAlloc.memoryTypeIndex = 0;
    VkMemoryRequirements memReqs;

    VkResult err;
    void *data;

    // Generate vertex buffer
    //	Setup
    VkBufferCreateInfo bufInfo = {};
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.pNext = NULL;
    bufInfo.size = vertexBufferSize;
    bufInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufInfo.flags = 0;
    //	Copy vertex data to VRAM
    memset(&vertices, 0, sizeof(vertices));
    err = vkCreateBuffer(device, &bufInfo, nullptr, &vertices.buf);
    assert(!err);
    vkGetBufferMemoryRequirements(device, vertices.buf, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memAlloc.memoryTypeIndex);
    vkAllocateMemory(device, &memAlloc, nullptr, &vertices.mem);
    assert(!err);
    err = vkMapMemory(device, vertices.mem, 0, memAlloc.allocationSize, 0, &data);
    assert(!err);
    memcpy(data, vertexBuffer.data(), vertexBufferSize);
    vkUnmapMemory(device, vertices.mem);
    assert(!err);
    err = vkBindBufferMemory(device, vertices.buf, vertices.mem, 0);
    assert(!err);

    // Generate index buffer
    //	Setup
    VkBufferCreateInfo indexbufferInfo = {};
    indexbufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    indexbufferInfo.pNext = NULL;
    indexbufferInfo.size = indexBufferSize;
    indexbufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    indexbufferInfo.flags = 0;
    // Copy index data to VRAM
    memset(&indices, 0, sizeof(indices));
    err = vkCreateBuffer(device, &bufInfo, nullptr, &indices.buf);
    assert(!err);
    vkGetBufferMemoryRequirements(device, indices.buf, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memAlloc.memoryTypeIndex);
    err = vkAllocateMemory(device, &memAlloc, nullptr, &indices.mem);
    assert(!err);
    err = vkMapMemory(device, indices.mem, 0, indexBufferSize, 0, &data);
    assert(!err);
    memcpy(data, indexBuffer.data(), indexBufferSize);
    vkUnmapMemory(device, indices.mem);
    err = vkBindBufferMemory(device, indices.buf, indices.mem, 0);
    assert(!err);
    indices.count = indexBuffer.size();

    // Binding description
    vertices.bindingDescriptions.resize(1);
    vertices.bindingDescriptions[0].binding = VERTEX_BUFFER_BIND_ID;
    vertices.bindingDescriptions[0].stride = sizeof(Vertex);
    vertices.bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // Attribute descriptions
    // Describes memory layout and shader attribute locations
    vertices.attributeDescriptions.resize(2);
    // Location 0 : Position
    vertices.attributeDescriptions[0].binding = VERTEX_BUFFER_BIND_ID;
    vertices.attributeDescriptions[0].location = 0;
    vertices.attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertices.attributeDescriptions[0].offset = 0;
    vertices.attributeDescriptions[0].binding = 0;
    // Location 1 : Color
    vertices.attributeDescriptions[1].binding = VERTEX_BUFFER_BIND_ID;
    vertices.attributeDescriptions[1].location = 1;
    vertices.attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertices.attributeDescriptions[1].offset = sizeof(float) * 3;
    vertices.attributeDescriptions[1].binding = 0;

    // Assign to vertex buffer
    vertices.vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertices.vi.pNext = NULL;
    vertices.vi.vertexBindingDescriptionCount = vertices.bindingDescriptions.size();
    vertices.vi.pVertexBindingDescriptions = vertices.bindingDescriptions.data();
    vertices.vi.vertexAttributeDescriptionCount = vertices.attributeDescriptions.size();
    vertices.vi.pVertexAttributeDescriptions = vertices.attributeDescriptions.data();
}

void setupDescriptorPool()
{
    // We need to tell the API the number of max. requested descriptors per type
    VkDescriptorPoolSize typeCounts[1];
    // This example only uses one descriptor type (uniform buffer) and only
    // requests one descriptor of this type
    typeCounts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    typeCounts[0].descriptorCount = 1;
    // For additional types you need to add new entries in the type count list
    // E.g. for two combined image samplers :
    // typeCounts[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    // typeCounts[1].descriptorCount = 2;

    // Create the global descriptor pool
    // All descriptors used in this example are allocated from this pool
    VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.pNext = NULL;
    descriptorPoolInfo.poolSizeCount = 1;
    descriptorPoolInfo.pPoolSizes = typeCounts;
    // Set the max. number of sets that can be requested
    // Requesting descriptors beyond maxSets will result in an error
    descriptorPoolInfo.maxSets = 1;

    VkResult vkRes = vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool);
    assert(!vkRes);
}

void setupDescriptorSetLayout()
{
    // Setup layout of descriptors used in this example
    // Basically connects the different shader stages to descriptors
    // for binding uniform buffers, image samplers, etc.
    // So every shader binding should map to one descriptor set layout
    // binding

    // Binding 0 : Uniform buffer (Vertex shader)
    VkDescriptorSetLayoutBinding layoutBinding = {};
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layoutBinding.pImmutableSamplers = NULL;

    VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
    descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorLayout.pNext = NULL;
    descriptorLayout.bindingCount = 1;
    descriptorLayout.pBindings = &layoutBinding;

    VkResult err = vkCreateDescriptorSetLayout(device, &descriptorLayout, NULL, &descriptorSetLayout);
    assert(!err);

    // Create the pipeline layout that is used to generate the rendering pipelines that
    // are based on this descriptor set layout
    // In a more complex scenario you would have different pipeline layouts for different
    // descriptor set layouts that could be reused
    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
    pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pPipelineLayoutCreateInfo.pNext = NULL;
    pPipelineLayoutCreateInfo.setLayoutCount = 1;
    pPipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;

    err = vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout);
    assert(!err);
}

void setupDescriptorSet()
{
    // Update descriptor sets determining the shader binding points
    // For every binding point used in a shader there needs to be one
    // descriptor set matching that binding point
    VkWriteDescriptorSet writeDescriptorSet = {};

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    VkResult vkRes = vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);
    assert(!vkRes);

    // Binding 0 : Uniform buffer
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = descriptorSet;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.pBufferInfo = &uniformDataVS.descriptor;
    // Binds this uniform buffer to binding point 0
    writeDescriptorSet.dstBinding = 0;

    vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, NULL);
}

void preparePipelines()
{
    // Create our rendering pipeline used in this example
    // Vulkan uses the concept of rendering pipelines to encapsulate
    // fixed states
    // This replaces OpenGL's huge (and cumbersome) state machine
    // A pipeline is then stored and hashed on the GPU making
    // pipeline changes much faster than having to set dozens of 
    // states
    // In a real world application you'd have dozens of pipelines
    // for every shader set used in a scene
    // Note that there are a few states that are not stored with
    // the pipeline. These are called dynamic states and the 
    // pipeline only stores that they are used with this pipeline,
    // but not their states

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};

    VkResult err;

    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    // The layout used for this pipeline
    pipelineCreateInfo.layout = pipelineLayout;
    // Renderpass this pipeline is attached to
    pipelineCreateInfo.renderPass = renderPass;

    // Vertex input state
    // Describes the topoloy used with this pipeline
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    // This pipeline renders vertex data as triangle lists
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // Rasterization state
    VkPipelineRasterizationStateCreateInfo rasterizationState = {};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    // Solid polygon mode
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    // No culling
    rasterizationState.cullMode = VK_CULL_MODE_NONE;
    rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationState.depthClampEnable = VK_FALSE;
    rasterizationState.rasterizerDiscardEnable = VK_FALSE;
    rasterizationState.depthBiasEnable = VK_FALSE;

    // Color blend state
    // Describes blend modes and color masks
    VkPipelineColorBlendStateCreateInfo colorBlendState = {};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    // One blend attachment state
    // Blending is not used in this example
    VkPipelineColorBlendAttachmentState blendAttachmentState[1] = {};
    blendAttachmentState[0].colorWriteMask = 0xf;
    blendAttachmentState[0].blendEnable = VK_FALSE;
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = blendAttachmentState;

    // Viewport state
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    // One viewport
    viewportState.viewportCount = 1;
    // One scissor rectangle
    viewportState.scissorCount = 1;

    // Enable dynamic states
    // Describes the dynamic states to be used with this pipeline
    // Dynamic states can be set even after the pipeline has been created
    // So there is no need to create new pipelines just for changing
    // a viewport's dimensions or a scissor box
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    // The dynamic state properties themselves are stored in the command buffer
    std::vector<VkDynamicState> dynamicStateEnables;
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pDynamicStates = dynamicStateEnables.data();
    dynamicState.dynamicStateCount = dynamicStateEnables.size();

    // Depth and stencil state
    // Describes depth and stenctil test and compare ops
    VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
    // Basic depth compare setup with depth writes and depth test enabled
    // No stencil used 
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.depthTestEnable = VK_TRUE;
    depthStencilState.depthWriteEnable = VK_TRUE;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilState.depthBoundsTestEnable = VK_FALSE;
    depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
    depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
    depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
    depthStencilState.stencilTestEnable = VK_FALSE;
    depthStencilState.front = depthStencilState.back;

    // Multi sampling state
    VkPipelineMultisampleStateCreateInfo multisampleState = {};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.pSampleMask = NULL;
    // No multi sampling used in this example
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Load shaders
    VkPipelineShaderStageCreateInfo shaderStages[2] = { {},{} };

#define USE_GLSL 1
#ifdef USE_GLSL
    shaderStages[0] = loadShaderGLSL("shaders/triangle.vert", VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = loadShaderGLSL("shaders/triangle.frag", VK_SHADER_STAGE_FRAGMENT_BIT);
#else
    shaderStages[0] = loadShader("shaders/triangle.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = loadShader("shaders/triangle.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
#endif

    // Assign states
    // Two shader stages
    pipelineCreateInfo.stageCount = 2;
    // Assign pipeline state create information
    pipelineCreateInfo.pVertexInputState = &vertices.vi;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDepthStencilState = &depthStencilState;
    pipelineCreateInfo.pStages = shaderStages;
    pipelineCreateInfo.renderPass = renderPass;
    pipelineCreateInfo.pDynamicState = &dynamicState;

    // Create rendering pipeline
    err = vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipelines.solid);
    assert(!err);
}

void prepareUniformBuffers()
{
    // Prepare and initialize uniform buffer containing shader uniforms
    VkMemoryRequirements memReqs;

    // Vertex shader uniform buffer block
    VkBufferCreateInfo bufferInfo = {};
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = NULL;
    allocInfo.allocationSize = 0;
    allocInfo.memoryTypeIndex = 0;
    VkResult err;

    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(uboVS);
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    // Create a new buffer
    err = vkCreateBuffer(device, &bufferInfo, nullptr, &uniformDataVS.buffer);
    assert(!err);
    // Get memory requirements including size, alignment and memory type 
    vkGetBufferMemoryRequirements(device, uniformDataVS.buffer, &memReqs);
    allocInfo.allocationSize = memReqs.size;
    // Gets the appropriate memory type for this type of buffer allocation
    // Only memory types that are visible to the host
    getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &allocInfo.memoryTypeIndex);
    // Allocate memory for the uniform buffer
    err = vkAllocateMemory(device, &allocInfo, nullptr, &(uniformDataVS.memory));
    assert(!err);
    // Bind memory to buffer
    err = vkBindBufferMemory(device, uniformDataVS.buffer, uniformDataVS.memory, 0);
    assert(!err);

    // Store information in the uniform's descriptor
    uniformDataVS.descriptor.buffer = uniformDataVS.buffer;
    uniformDataVS.descriptor.offset = 0;
    uniformDataVS.descriptor.range = sizeof(uboVS);

    updateUniformBuffers();
}

void updateUniformBuffers()
{
    // Update matrices
    uboVS.projectionMatrix = glm::perspective(deg_to_rad(60.0f), (float)width / (float)height, 0.1f, 256.0f);

    uboVS.viewMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, zoom));

    uboVS.modelMatrix = glm::mat4();
    uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, deg_to_rad(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, deg_to_rad(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, deg_to_rad(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    // Map uniform buffer and update it
    uint8_t *pData;
    VkResult err = vkMapMemory(device, uniformDataVS.memory, 0, sizeof(uboVS), 0, (void **)&pData);
    assert(!err);
    memcpy(pData, &uboVS, sizeof(uboVS));
    vkUnmapMemory(device, uniformDataVS.memory);
    assert(!err);
}

void render()
{
    vkDeviceWaitIdle(device);
    draw();
    vkDeviceWaitIdle(device);

}

void viewChanged()
{
    // This function is called by the base example class 
    // each time the view is changed by user input
    updateUniformBuffers();
}


VkPipelineShaderStageCreateInfo loadShader(const char * fileName, VkShaderStageFlagBits stage)
{
    VkPipelineShaderStageCreateInfo shaderStage = {};
    shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.stage = stage;
    shaderStage.module = vkTools::loadShader(fileName, device, stage);
    shaderStage.pName = "main"; // todo : make param
    assert(shaderStage.module != NULL);
    shaderModules.push_back(shaderStage.module);
    return shaderStage;
}

VkPipelineShaderStageCreateInfo loadShaderGLSL(const char * fileName, VkShaderStageFlagBits stage)
{
    VkPipelineShaderStageCreateInfo shaderStage = {};
    shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.stage = stage;
    shaderStage.module = vkTools::loadShaderGLSL(fileName, device, stage);
    shaderStage.pName = "main"; // todo : make param
    assert(shaderStage.module != NULL);
    shaderModules.push_back(shaderStage.module);
    return shaderStage;
}

VkBool32 createBuffer(
    VkBufferUsageFlags usage,
    VkDeviceSize size,
    void * data,
    VkBuffer *buffer,
    VkDeviceMemory *memory)
{
    VkMemoryRequirements memReqs;
    VkMemoryAllocateInfo memAlloc = vkTools::initializers::memoryAllocateInfo();
    VkBufferCreateInfo bufferCreateInfo = vkTools::initializers::bufferCreateInfo(usage, size);

    VkResult err = vkCreateBuffer(device, &bufferCreateInfo, nullptr, buffer);
    assert(!err);
    vkGetBufferMemoryRequirements(device, *buffer, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memAlloc.memoryTypeIndex);
    err = vkAllocateMemory(device, &memAlloc, nullptr, memory);
    assert(!err);
    if (data != nullptr)
    {
        void *mapped;
        err = vkMapMemory(device, *memory, 0, size, 0, &mapped);
        assert(!err);
        memcpy(mapped, data, size);
        vkUnmapMemory(device, *memory);
    }
    err = vkBindBufferMemory(device, *buffer, *memory, 0);
    assert(!err);
    return true;
}

VkBool32 createBuffer(VkBufferUsageFlags usage, VkDeviceSize size, void * data, VkBuffer * buffer, VkDeviceMemory * memory, VkDescriptorBufferInfo * descriptor)
{
    VkBool32 res = createBuffer(usage, size, data, buffer, memory);
    if (res)
    {
        descriptor->offset = 0;
        descriptor->buffer = *buffer;
        descriptor->range = size;
        return true;
    }
    else
    {
        return false;
    }
}

VkBool32 getMemoryType(uint32_t typeBits, VkFlags properties, uint32_t * typeIndex)
{
    for (uint32_t i = 0; i < 32; i++)
    {
        if ((typeBits & 1) == 1)
        {
            if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                *typeIndex = i;
                return true;
            }
        }
        typeBits >>= 1;
    }
    return false;
}


void createCommandPool()
{
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = swapChain.queueNodeIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VkResult vkRes = vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &cmdPool);
    assert(!vkRes);
}

void setupDepthStencil()
{
    VkImageCreateInfo image = {};
    image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image.pNext = NULL;
    image.imageType = VK_IMAGE_TYPE_2D;
    image.format = depthFormat;
    image.extent = { width, height, 1 };
    image.mipLevels = 1;
    image.arrayLayers = 1;
    image.samples = VK_SAMPLE_COUNT_1_BIT;
    image.tiling = VK_IMAGE_TILING_OPTIMAL;
    image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image.flags = 0;

    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = NULL;
    mem_alloc.allocationSize = 0;
    mem_alloc.memoryTypeIndex = 0;

    VkImageViewCreateInfo depthStencilView = {};
    depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    depthStencilView.pNext = NULL;
    depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depthStencilView.format = depthFormat;
    depthStencilView.flags = 0;
    depthStencilView.subresourceRange = {};
    depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    depthStencilView.subresourceRange.baseMipLevel = 0;
    depthStencilView.subresourceRange.levelCount = 1;
    depthStencilView.subresourceRange.baseArrayLayer = 0;
    depthStencilView.subresourceRange.layerCount = 1;

    VkMemoryRequirements memReqs;
    VkResult err;

    err = vkCreateImage(device, &image, nullptr, &depthStencil.image);
    assert(!err);
    vkGetImageMemoryRequirements(device, depthStencil.image, &memReqs);
    mem_alloc.allocationSize = memReqs.size;
    getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mem_alloc.memoryTypeIndex);
    err = vkAllocateMemory(device, &mem_alloc, nullptr, &depthStencil.mem);
    assert(!err);

    err = vkBindImageMemory(device, depthStencil.image, depthStencil.mem, 0);
    assert(!err);
    vkTools::setImageLayout(setupCmdBuffer, depthStencil.image, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    depthStencilView.image = depthStencil.image;
    err = vkCreateImageView(device, &depthStencilView, nullptr, &depthStencil.view);
    assert(!err);
}

void setupFrameBuffer()
{
    VkImageView attachments[2];

    // Depth/Stencil attachment is the same for all frame buffers
    attachments[1] = depthStencil.view;

    VkFramebufferCreateInfo frameBufferCreateInfo = {};
    frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferCreateInfo.pNext = NULL;
    frameBufferCreateInfo.renderPass = renderPass;
    frameBufferCreateInfo.attachmentCount = 2;
    frameBufferCreateInfo.pAttachments = attachments;
    frameBufferCreateInfo.width = width;
    frameBufferCreateInfo.height = height;
    frameBufferCreateInfo.layers = 1;

    // Create frame buffers for every swap chain image
    frameBuffers.resize(swapChain.imageCount);
    for (uint32_t i = 0; i < frameBuffers.size(); i++)
    {
        attachments[0] = swapChain.buffers[i].view;
        VkResult err = vkCreateFramebuffer(device, &frameBufferCreateInfo, nullptr, &frameBuffers[i]);
        assert(!err);
    }
}

void setupRenderPass()
{
    VkAttachmentDescription attachments[2];
    attachments[0].format = colorformat;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    attachments[1].format = depthFormat;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorReference = {};
    colorReference.attachment = 0;
    colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthReference = {};
    depthReference.attachment = 1;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.flags = 0;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = NULL;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorReference;
    subpass.pResolveAttachments = NULL;
    subpass.pDepthStencilAttachment = &depthReference;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = NULL;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = NULL;
    renderPassInfo.attachmentCount = 2;
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 0;
    renderPassInfo.pDependencies = NULL;

    VkResult err;

    err = vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);
    assert(!err);
}

void setupSwapChain()
{
    swapChain.setup(setupCmdBuffer, &width, &height);
}