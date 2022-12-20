/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "get_intel_product_vk.h"

#include <volk.h>

IntelProduct getIntelProduct(VkPhysicalDevice physicalDevice) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    return getIntelProduct(static_cast<uint32_t>(properties.deviceID));
}

IntelProduct getIntelProduct(const Vulkan &vulkan) {
    return getIntelProduct(vulkan.physicalDevice);
}
