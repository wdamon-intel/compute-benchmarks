/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "framework/vk/vk.h"

#include <atomic>
#include <vector>
#include <volk.h>

namespace VK {

class CommandPool {
  public:
    CommandPool();
    ~CommandPool();

    VkResult init(VkDevice device, VkQueue queue, uint32_t count, uint32_t queueIndex, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    void destroy();

    VkResult beginPrimary(uint32_t cmdBufIndex, VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    VkResult flush(uint32_t cmdBufIndex, bool waitForComplete = false);
    VkResult flushAll(bool waitForComplete = false);

    // Accessors
    const VkCommandBuffer *commandBuffers() const;
    VkCommandBuffer commandBufferAt(uint32_t index) const;
    uint32_t numCommandBuffers() const;

  private:
    VkResult end(uint32_t cmdBufIndex);

  private:
    VkDevice                     _device         { nullptr };
    VkQueue                      _queue          { nullptr };
    VkCommandPool                _commandPool    { nullptr };
    std::vector<VkCommandBuffer> _commandBuffers {         };
    uint32_t                     _began          {    0    };
};

} // namespace VK
