/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "framework/vk/utility/vk_buffer.h"

#include "framework/utility/buffer_contents_helper.h"
#include "framework/vk/utility/error.h"
#include "framework/vk/vulkan.h"

#include <volk.h>

namespace VK {

static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);

    for (uint32_t index = 0; index < physicalDeviceMemoryProperties.memoryTypeCount; ++index) {
        if ((typeFilter & (1U << index)) && (properties == (physicalDeviceMemoryProperties.memoryTypes[index].propertyFlags & properties))) {
            return index;
        }
    }

    FATAL_ERROR(std::string("failed to find suitable memory type"));
}

Buffer::Buffer(Vulkan *vulkan, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode, VkMemoryPropertyFlags properties)
    : _vulkan(vulkan), _buffer(nullptr), _memory(nullptr), _requestedSizeInBytes(size), _actualSizeInBytes(0), _stagingBuffer() {
    VkBufferCreateInfo createInfo;
    createInfo.sType                    = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.pNext                    = NULL;
    createInfo.flags                    = 0;
    createInfo.size                     = _requestedSizeInBytes;
    createInfo.usage                    = usage;
    createInfo.sharingMode              = sharingMode;
    createInfo.queueFamilyIndexCount    = 0;
    createInfo.pQueueFamilyIndices      = NULL;

    VK_SUCCESS_OR_ERROR(vkCreateBuffer(_vulkan->device, &createInfo, NULL, &_buffer), "Buffer::Buffer(...): Failed to create buffer object");

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(_vulkan->device, _buffer, &requirements);

    VkMemoryAllocateInfo allocInfo;
    allocInfo.sType                     = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext                     = NULL;
    allocInfo.allocationSize            = requirements.size;
    allocInfo.memoryTypeIndex           = findMemoryType(_vulkan->physicalDevice, requirements.memoryTypeBits, properties);

    VK_SUCCESS_OR_ERROR(vkAllocateMemory(_vulkan->device, &allocInfo, NULL, &_memory), "Buffer::Buffer(...): Failed to allocate buffer memory");

    VK_SUCCESS_OR_ERROR(vkBindBufferMemory(_vulkan->device, _buffer, _memory, 0), "Buffer::Buffer(...): Failed to bind buffer memory to buffer object");

    _actualSizeInBytes = requirements.size;
}

Buffer::~Buffer() {
    _stagingBuffer.reset();

    if (_buffer != nullptr) {
        vkDestroyBuffer(_vulkan->device, _buffer, NULL);
        _buffer = nullptr;
    }
    if (_memory != nullptr) {
        vkFreeMemory(_vulkan->device, _memory, NULL);
        _memory = nullptr;
    }
    _vulkan = nullptr;
}

VkResult Buffer::fill(BufferContents contents, bool discardStagingBuffer) {
    if (!_stagingBuffer) {
        VK_SUCCESS_OR_RETURN(buildStagingBuffer(contents));
    }

    copyFrom(*_stagingBuffer, 0, 0, _requestedSizeInBytes, true /* waitForComplete */);

    if (discardStagingBuffer) {
        _stagingBuffer.reset();
    }

    return VK_SUCCESS;
}

VkResult Buffer::map(VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void **ppData) {
    return vkMapMemory(_vulkan->device, _memory, offset, size, flags, ppData);
}

void Buffer::unmap() {
    vkUnmapMemory(_vulkan->device, _memory);
}

VkResult Buffer::copyFrom(const Buffer &srcBuffer, VkDeviceSize srcOffset, VkDeviceSize dstOffset, VkDeviceSize size, bool waitForComplete) {
    uint32_t cmdBufIdx = 0;
    EndEncodingFlags cmdBufFlags = (waitForComplete ? EndEncodingFlags::FINISH : EndEncodingFlags::NONE);
    VkCommandBuffer cmdBuf = _vulkan->commandBuffer(&cmdBufIdx);

    VkBufferCopy copyRegion;
    copyRegion.srcOffset = srcOffset;
    copyRegion.dstOffset = dstOffset;
    copyRegion.size      = size;

    vkCmdCopyBuffer(cmdBuf, srcBuffer.getBuffer(), _buffer, 1, &copyRegion);

    _vulkan->endEncoding(cmdBufIdx, cmdBufFlags);

    return VK_SUCCESS;
}

VkResult Buffer::buildStagingBuffer(BufferContents contents) {
    _stagingBuffer = std::make_shared<Buffer>(_vulkan, _requestedSizeInBytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    uint8_t *data = nullptr;
    VK_SUCCESS_OR_RETURN(_stagingBuffer->map(0, _requestedSizeInBytes, 0, reinterpret_cast<void **>(&data)));
    switch (contents) {
    case BufferContents::Random:
        BufferContentsHelper::fillWithRandomBytes(data, _requestedSizeInBytes);
        break;
    case BufferContents::IncreasingBytes:
        BufferContentsHelper::fillWithIncreasingBytes(data, _requestedSizeInBytes);
        break;
    case BufferContents::Zeros:
        // fallthrough
    case BufferContents::Unknown:
        // fallthrough
    default:
        BufferContentsHelper::fillWithZeros(data, _requestedSizeInBytes);
        break;
    }
    _stagingBuffer->unmap();

    return VK_SUCCESS;
}

} // namespace VK
