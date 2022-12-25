/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "framework/test_case/test_case.h"
#include "framework/vk/vk.h"
#include "framework/vk/context_properties.h"
#include "framework/vk/queue_properties.h"

#include <vector>

#include <volk.h>

namespace VK {

class CommandPool;

// Class handles regular Vulkan boilerplate code, such as creating instances, devices, contexts,
// queues, etc...  It is configurable by the QueueProperties and ContextProperties objects,
// allowing benchmark applications to set perform the setup in a different way than usual. The
// default constructor simply creates and uses the default device.

struct Vulkan {
    VkInstance       instance       {};
    VkPhysicalDevice physicalDevice {};
    VkDevice         device         {};
    VkQueue          graphicsQueue  {};
    VkQueue          computeQueue   {};
    VkQueue          transferQueue  {};
    VkQueue          presentQueue   {};

    Vulkan() : Vulkan(QueueProperties::create()) {}
    Vulkan(const QueueProperties &queueProperties) : Vulkan(queueProperties, ContextProperties::create()) {}
    Vulkan(const QueueProperties &queueProperties, const ContextProperties &contextProperties);
    ~Vulkan();

private:
    enum class Limits : uint32_t
    {
        MAX_ENABLED_EXTENSIONS = 64
    };

    void _selectInstanceLayers(const QueueProperties &queueProperties, const ContextProperties &contextProperties);
    void _selectInstanceExtensions(const QueueProperties &queueProperties, const ContextProperties &contextProperties);
    void _createInstance(const QueueProperties &queueProperties, const ContextProperties &contextProperties);

    void _selectPhysicalDevice(const QueueProperties &queueProperties, const ContextProperties &contextProperties);
    void _selectDeviceExtensions(const QueueProperties &queueProperties, const ContextProperties &contextProperties);
    void _selectQueueFamilies(const QueueProperties &queueProperties, const ContextProperties &contextProperties);
    void _createDevice(const QueueProperties &queueProperties, const ContextProperties &contextProperties);
    void _createQueues(const QueueProperties &queueProperties, const ContextProperties &contextProperties);

    std::vector<const char*> enabledInstanceExtensions  {};
    std::vector<const char*> enabledInstanceLayers      {};
    std::vector<const char*> enabledDeviceExtensions    {};

    std::unique_ptr<CommandPool> _commandPool           {};
};

} // namespace VK

using namespace VK;