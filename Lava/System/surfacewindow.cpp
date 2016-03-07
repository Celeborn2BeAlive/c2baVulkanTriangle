#include "surfacewindow.hpp"
#include "Vulkan/exception.hpp"
#include <cassert>
#include <unordered_map>
#include <utility>

std::unordered_map<GLFWwindow*, SurfaceWindow*> windowVector;

void resizeSurface(GLFWwindow *win, int w, int h) {
    assert(windowVector.find(win) != windowVector.end());
    windowVector[win]->resize(w, h);
}

SurfaceWindow::SurfaceWindow(Instance &instance, Device &device, int width, int height, const char *title) :
    mInstance(instance), mDevice(device), mWidth(width), mHeight(height) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    mWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
    assert(mWindow != nullptr);
    vulkanCheckError(glfwCreateWindowSurface(instance, mWindow, nullptr, &mSurface));

    windowVector[mWindow] = this;
    glfwSetWindowSizeCallback(mWindow, resizeSurface);

    createSwapchain();
}

bool SurfaceWindow::isRunning() const {
    return !glfwWindowShouldClose(mWindow);
}

void SurfaceWindow::updateEvent() const {
    glfwPollEvents();
}

void SurfaceWindow::initFrameBuffers() {
    VkImage images[2];
    uint32_t nImg = 2;

    vkGetSwapchainImagesKHR(mDevice, mSwapchain, &nImg, images);

    for(auto i(0u); i < nImg; ++i) {
        std::vector<ImageView> allViews;
        allViews.emplace_back(mDevice, images[i], mFormat);
        mFrameBuffers[i] = std::make_unique<FrameBuffer>(mDevice, *mRenderPass, std::move(allViews), mWidth, mHeight, 1);
    }
}

void SurfaceWindow::createSwapchain() {
    VkSwapchainCreateInfoKHR info;

    uint32_t nFormat;
    vkGetPhysicalDeviceSurfaceFormatsKHR(mDevice, mSurface, &nFormat, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(nFormat);
    vkGetPhysicalDeviceSurfaceFormatsKHR(mDevice, mSurface, &nFormat, &formats[0]);

    if(nFormat == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
        formats[0].format = VK_FORMAT_B8G8R8A8_SRGB;

    mFormat = formats[0].format;
    mRenderPass = std::make_unique<RenderPass>(mDevice, mFormat);

    info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    info.pNext = nullptr;
    info.flags = 0;
    info.imageFormat = formats[0].format;
    info.imageColorSpace = formats[0].colorSpace;
    info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    info.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    info.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    info.surface = mSurface;
    info.minImageCount = 2; // Double buffering...
    info.imageExtent.width = mWidth;
    info.imageExtent.height = mHeight;

    vulkanCheckError(vkCreateSwapchainKHR(mDevice, &info, nullptr, &mSwapchain));
    initFrameBuffers();
}

void SurfaceWindow::resize(int w, int h) {
    mWidth = w;
    mHeight = h;

    vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
    createSwapchain();
}

void SurfaceWindow::begin() {
    // No checking because could be in lost state if change res
    vkAcquireNextImageKHR(mDevice, mSwapchain, UINT64_MAX, VK_NULL_HANDLE, VK_NULL_HANDLE, &mCurrentSwapImage);
}

void SurfaceWindow::end(Queue &queue) {
    VkPresentInfoKHR info;

    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.pNext = nullptr;
    info.waitSemaphoreCount = 0;
    info.pWaitSemaphores = nullptr;
    info.swapchainCount = 1;
    info.pSwapchains = &mSwapchain;
    info.pImageIndices = &mCurrentSwapImage;
    info.pResults = nullptr;

    vkQueuePresentKHR(queue, &info);
}

VkRenderPass SurfaceWindow::mainRenderPass() const {
    return *mRenderPass;
}

VkFramebuffer SurfaceWindow::getCurrentFrameBuffer() const {
    return *mFrameBuffers[mCurrentSwapImage];
}

int SurfaceWindow::width() const {
    return mWidth;
}

int SurfaceWindow::height() const {
    return mHeight;
}

SurfaceWindow::~SurfaceWindow() {
    vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
    vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
}
