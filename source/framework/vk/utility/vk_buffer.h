/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "framework/enum/buffer_contents.h"
#include "framework/vk/vk.h"

#include <memory>

namespace VK {

struct Vulkan;

class Buffer {
  public:
    explicit Buffer(Vulkan *vulkan, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode, VkMemoryPropertyFlags properties);
    ~Buffer();

    // Non-copyable
    Buffer() = delete;
    Buffer(const Buffer &) = delete;
    Buffer(Buffer &&) = delete;
    Buffer &operator=(const Buffer &) = delete;

    VkResult fill(BufferContents contents, bool discardStagingBuffer = true);

    VkResult map(VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void **ppData);
    void unmap();

    VkResult copyFrom(const Buffer &srcBuffer, VkDeviceSize srcOffset, VkDeviceSize dstOffset, VkDeviceSize size, bool waitForComplete = false);

  protected:
    inline VkBuffer getBuffer() const { return _buffer; }

  private:
    VkResult buildStagingBuffer(BufferContents contents);

  private:
    Vulkan*                 _vulkan;
    VkBuffer                _buffer;
    VkDeviceMemory          _memory;
    VkDeviceSize            _requestedSizeInBytes;
    VkDeviceSize            _actualSizeInBytes;
    std::shared_ptr<Buffer> _stagingBuffer;
};

} // namespace VK
