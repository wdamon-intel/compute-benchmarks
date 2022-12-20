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

namespace VK {

// Class handles regular Vulkan boilerplate code, such as creating instances, devices, contexts,
// queues, etc...  It is configurable by the QueueProperties and ContextProperties objects,
// allowing benchmark applications to set perform the setup in a different way than usual. The
// default constructor simply creates and uses the default device.

struct Vulkan {
    VkInstance       instance       {};
    VkPhysicalDevice physicalDevice {};
    VkDevice         device         {};
    VkQueue          graphicsQueue  {};
    //VkQueue computeQueue{};
    //VkQueue presentQueue{};

    Vulkan() : Vulkan(QueueProperties::create()) {}
    Vulkan(const QueueProperties &queueProperties) : Vulkan(queueProperties, ContextProperties::create()) {}
    Vulkan(const QueueProperties &queueProperties, const ContextProperties &contextProperties);
    ~Vulkan();
};

} // namespace VK

using namespace VK;