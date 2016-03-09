#include "System/contextinitializer.hpp"
#include "System/Vulkan/instance.hpp"
#include "System/Vulkan/physicaldevices.hpp"
#include "System/Vulkan/device.hpp"
#include "System/Vulkan/queue.hpp"
#include "System/surfacewindow.hpp"
#include "System/Vulkan/exception.hpp"
#include "System/Vulkan/commandpool.hpp"
#include "System/Vulkan/fence.hpp"

int main()
{
    ContextInitializer context;
    Instance instance(context.getExtensionNumber(), context.getExtensions());
    PhysicalDevices physicalDevices(instance);
    Device device(physicalDevices, 0, {1.f}, 1);
    Queue queue(device, 0, 0);

    SurfaceWindow window(instance, device, 800, 600, "Lava");

    CommandPool commandPool(device, 0);

    Fence fence(device, 1);

    VkCommandBuffer commandBuffer;
    VkCommandBufferAllocateInfo allocateInfo;

    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = commandPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = 1;

    vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer);

    float v = 0.0f;
    while(window.isRunning()) {
        window.updateEvent();

        v += 0.00001;

        window.begin();
        VkCommandBufferBeginInfo bi;
        VkRenderPassBeginInfo ri;

        VkClearValue c;
        c.color.float32[0] = v; c.color.float32[1] = c.color.float32[2] = 0.3;
        c.color.float32[3] = 1.0;
        bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        bi.flags = 0;

        ri.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        ri.renderPass = window.mainRenderPass();
        ri.framebuffer = window.getCurrentFrameBuffer();
        ri.renderArea = VkRect2D{{0, 0}, {uint32_t(window.width()), uint32_t(window.height())}};
        ri.clearValueCount = 1;
        ri.pClearValues = &c;

        vkBeginCommandBuffer(commandBuffer, &bi);
        vkCmdBeginRenderPass(commandBuffer, &ri, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport;
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = window.width();
        viewport.height = window.height();
        viewport.minDepth = 0;
        viewport.maxDepth = 1;

         vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

         vkCmdEndRenderPass(commandBuffer);
         vkEndCommandBuffer(commandBuffer);

         VkSubmitInfo vi;
         memset(&vi, 0, sizeof(vi));

         vi.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
         vi.commandBufferCount = 1;
         vi.pCommandBuffers = &commandBuffer;

         vulkanCheckError(vkQueueSubmit(queue, 1, &vi, fence.getFence(0)));

         fence.wait();

         window.end(queue);
    }

    glfwTerminate();

    return 0;
}
