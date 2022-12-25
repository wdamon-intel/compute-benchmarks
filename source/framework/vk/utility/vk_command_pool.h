/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "framework/vk/vk.h"

#include <volk.h>

#include <vector>

namespace VK {

class CommandPool
{
public:
    CommandPool();
    ~CommandPool();

    VkResult init(
        VkDevice             device,
        VkQueue              queue,
        uint32_t             count,
        uint32_t             queueIndex,
        VkCommandBufferLevel level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY
    );

    void destroy();

    VkResult beginPrimary(
        uint32_t                  cmdBufIndex,
        VkCommandBufferUsageFlags flags         = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
    );

    VkResult end(uint32_t cmdBufIndex);

    VkResult flush(uint32_t cmdBufIndex, bool waitForComplete = false);

    VkResult flushAll(bool waitForComplete = false);

    // Accessors
    const VkCommandBuffer* commandBuffers()                const;
    VkCommandBuffer        commandBufferAt(uint32_t index) const;
    uint32_t               numCommandBuffers()             const;

private:
    VkDevice                     _device            { nullptr };
    VkQueue                      _queue             { nullptr };
    VkCommandPool                _commandPool       { nullptr };
    std::vector<VkCommandBuffer> _commandBuffers    {         };
    bool                         _began             {  false  };
};

} // namespace VK
