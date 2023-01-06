
#include "framework/vk/utility/vk_command_pool.h"
#include "framework/vk/utility/error.h"

#include <string>

namespace VK {

CommandPool::CommandPool()
{
    // void
}

CommandPool::~CommandPool()
{
    destroy();
}

void CommandPool::destroy()
{
    _began = 0;

    if (_commandBuffers.size() > 0)
    {
        vkFreeCommandBuffers(_device, _commandPool, static_cast<uint32_t>(_commandBuffers.size()), _commandBuffers.data());
        _commandBuffers.clear();
    }

    if (_commandPool != nullptr)
    {
        vkDestroyCommandPool(_device, _commandPool, NULL);
        _commandPool = nullptr;
    }

    _queue  = nullptr;
    _device = nullptr;
}

VkResult CommandPool::init(
    VkDevice             device,
    VkQueue              queue,
    uint32_t             count,
    uint32_t             queueIndex,
    VkCommandBufferLevel level
)
{
    VK_CHECK_PARAMETER_OR_RETURN((device != nullptr), "device must be valid");
    VK_CHECK_PARAMETER_OR_RETURN((count   > 0      ), "invalid count for command pool");

    _device = device;
    _queue  = queue;
    _commandBuffers.resize(count);


    VkCommandPoolCreateInfo cpCreateInfo;
    cpCreateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cpCreateInfo.pNext              = NULL;
    cpCreateInfo.queueFamilyIndex   = queueIndex;
    cpCreateInfo.flags              = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VK_SUCCESS_OR_RETURN(vkCreateCommandPool(_device, &cpCreateInfo, NULL, &_commandPool));

    VkCommandBufferAllocateInfo cbAllocInfo;
    cbAllocInfo.sType               = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cbAllocInfo.pNext               = NULL;
    cbAllocInfo.commandPool         = _commandPool;
    cbAllocInfo.level               = level;
    cbAllocInfo.commandBufferCount  = count;

    VK_SUCCESS_OR_RETURN(vkAllocateCommandBuffers(_device, &cbAllocInfo, _commandBuffers.data()));

    return VK_SUCCESS;
}

VkResult CommandPool::beginPrimary(
    uint32_t                  cmdBufIndex,
    VkCommandBufferUsageFlags flags
)
{
    const uint32_t cmdBufIndexBit = (1U << cmdBufIndex);

    VK_CHECK_PARAMETER_OR_RETURN((cmdBufIndex < _commandBuffers.size()), "invalid command buffer index");
    
    if (0 != (_began & cmdBufIndexBit))
    {
        return VK_ERROR_OUT_OF_POOL_MEMORY;
    }

    VkCommandBufferBeginInfo cbBeginInfo;
    cbBeginInfo.sType               = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cbBeginInfo.pNext               = NULL;
    cbBeginInfo.flags               = flags;
    cbBeginInfo.pInheritanceInfo    = NULL;

    VK_SUCCESS_OR_RETURN(vkBeginCommandBuffer(_commandBuffers[cmdBufIndex], &cbBeginInfo));

    _began |= cmdBufIndexBit;

    return VK_SUCCESS;
}

VkResult CommandPool::end(uint32_t cmdBufIndex)
{
    const uint32_t cmdBufIndexBit = (1U << cmdBufIndex);

    VK_CHECK_PARAMETER_OR_RETURN((0 != (_began & cmdBufIndexBit)), "cannot end command buffer, begin must be called first");
    VK_CHECK_PARAMETER_OR_RETURN((cmdBufIndex < _commandBuffers.size()), "invalid command buffer index");

    VK_SUCCESS_OR_RETURN(vkEndCommandBuffer(_commandBuffers[cmdBufIndex]));

    _began &= ~(cmdBufIndexBit);

    return VK_SUCCESS;
}

VkResult CommandPool::flush(uint32_t cmdBufIndex, bool waitForComplete)
{
    VkFence fence = nullptr;

    VK_SUCCESS_OR_RETURN(end(cmdBufIndex));

    VkSubmitInfo submitInfo;
    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext                = NULL;
    submitInfo.waitSemaphoreCount   = 0;
    submitInfo.pWaitSemaphores      = NULL;
    submitInfo.pWaitDstStageMask    = NULL;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &_commandBuffers[cmdBufIndex];
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores    = NULL;

    if (waitForComplete)
    {
        VkFenceCreateInfo fenceCreateInfo;
        fenceCreateInfo.sType           = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.pNext           = NULL;
        fenceCreateInfo.flags           = 0;

        VK_SUCCESS_OR_RETURN(vkCreateFence(_device, &fenceCreateInfo, NULL, &fence));
    }

    VK_SUCCESS_OR_RETURN(vkQueueSubmit(_queue, 1, &submitInfo, fence));

    if (waitForComplete)
    {
        VK_SUCCESS_OR_RETURN(vkWaitForFences(_device, 1, &fence, VK_TRUE, UINT64_MAX));
        vkDestroyFence(_device, fence, NULL);
    }

    return VK_SUCCESS;
}

VkResult CommandPool::flushAll(bool waitForComplete)
{
    VkResult retVal = VK_SUCCESS;
    for (size_t cmdBufIndex = 0; cmdBufIndex < _commandBuffers.size(); ++cmdBufIndex)
    {
        retVal = flush(static_cast<uint32_t>(cmdBufIndex), waitForComplete);
        if (retVal != VK_SUCCESS)
        {
            break;
        }
    }
    return retVal;
}

const VkCommandBuffer* CommandPool::commandBuffers() const
{
    return _commandBuffers.data();
}

VkCommandBuffer CommandPool::commandBufferAt(uint32_t index) const
{
    VK_CHECK_PARAMETER_OR_RETURN_NULL((index < _commandBuffers.size()), "invalid command buffer index");
    return _commandBuffers[index];
}

uint32_t CommandPool::numCommandBuffers() const
{
    return static_cast<uint32_t>(_commandBuffers.size());
}

} // namespace VK