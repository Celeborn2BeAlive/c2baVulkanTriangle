#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>
#include "Vulkan/renderpass.hpp"
#include "Vulkan/queue.hpp"
#include "loggable.hpp"
#include "Vulkan/framebuffer.hpp"

class SurfaceWindow : Loggable, NonCopyable
{
public:
    SurfaceWindow(Instance &instance, Device &device, int width, int height, char const *title);

    bool isRunning() const;
    void updateEvent() const;


    void resize(int w, int h);

    int width() const;
    int height() const;

    VkRenderPass mainRenderPass() const;
    VkFramebuffer getCurrentFrameBuffer() const;

    void begin();
    void end(Queue &queue);

    ~SurfaceWindow();

private:
    Instance &mInstance;
    Device &mDevice;
    int mWidth;
    int mHeight;

    GLFWwindow *mWindow;
    VkFormat mFormat;
    VkSurfaceKHR mSurface;
    VkSwapchainKHR mSwapchain;
    std::unique_ptr<RenderPass> mRenderPass;

    uint32_t mCurrentSwapImage = 0;
    std::unique_ptr<FrameBuffer> mFrameBuffers[2];

    void initFrameBuffers();
    void createSwapchain();
};
