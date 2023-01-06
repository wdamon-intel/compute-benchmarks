/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include <array>
#include <mutex>
#include <vector>

#include "framework/test_case/test_case.h"
#include "framework/vk/vk.h"
#include "framework/vk/context_properties.h"
#include "framework/vk/queue_properties.h"

#include <volk.h>

namespace VK {

class CommandPool;

enum class EndEncodingFlags : uint32_t {
    NONE        = 0x0, ///< End the encoding of the command buffer but do not submit it to hardware.
    FLUSH       = 0x1, ///< End the encoding of the command buffer, submit it to hardware but do not wait for completion.
    FINISH      = 0x2, ///< End the encoding of the command buffer, submit it to hardware and wait for completion.
};

// Class handles regular Vulkan boilerplate code, such as creating instances, devices, contexts,
// queues, etc...  It is configurable by the QueueProperties and ContextProperties objects,
// allowing benchmark applications to set perform the setup in a different way than usual. The
// default constructor simply creates and uses the default device.

struct Vulkan {
    VkInstance       instance       {};
    VkPhysicalDevice physicalDevice {};
    VkDevice         device         {};

    Vulkan() : Vulkan(QueueProperties::create()) {}
    Vulkan(const QueueProperties &queueProperties) : Vulkan(queueProperties, ContextProperties::create()) {}
    Vulkan(const QueueProperties &queueProperties, const ContextProperties &contextProperties);
    ~Vulkan();

    inline VkQueue graphicsQueue() const { return _queues[static_cast<uint32_t>(QueueIndex::GRAPHICS)]; }
 
    VkCommandBuffer commandBuffer(uint32_t *pIndex);
    void            endEncoding(uint32_t index, EndEncodingFlags flags = EndEncodingFlags::NONE);

 private:
    enum class Limits : uint32_t
    {
        MAX_ENABLED_EXTENSIONS = 64
    };

    enum class QueueIndex : uint32_t
    {
        GRAPHICS,
        COMPUTE,
        TRANSFER,
        PRESENT,

        QUEUE_INDEX_MAX
    };

    using QueueArray = std::array<VkQueue, static_cast<size_t>(QueueIndex::QUEUE_INDEX_MAX)>;

    void _selectInstanceLayers(const QueueProperties &queueProperties, const ContextProperties &contextProperties);
    void _selectInstanceExtensions(const QueueProperties &queueProperties, const ContextProperties &contextProperties);
    void _createInstance(const QueueProperties &queueProperties, const ContextProperties &contextProperties);

    void _selectPhysicalDevice(const QueueProperties &queueProperties, const ContextProperties &contextProperties);
    void _selectDeviceExtensions(const QueueProperties &queueProperties, const ContextProperties &contextProperties);
    void _selectQueueFamilies(const QueueProperties &queueProperties, const ContextProperties &contextProperties);
    void _createDevice(const QueueProperties &queueProperties, const ContextProperties &contextProperties);
    //void _createQueues(const QueueProperties &queueProperties, const ContextProperties &contextProperties);

    std::vector<const char*> enabledInstanceExtensions  {};
    std::vector<const char*> enabledInstanceLayers      {};
    std::vector<const char*> enabledDeviceExtensions    {};

    QueueArray                   _queues                {};
    std::unique_ptr<CommandPool> _commandPool           {};
    std::mutex                   _commandBufferMutex    {};
};

} // namespace VK

using namespace VK;