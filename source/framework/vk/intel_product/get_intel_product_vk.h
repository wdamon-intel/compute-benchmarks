/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "framework/intel_product/get_intel_product.h"
#include "framework/vk/vulkan.h"

IntelProduct getIntelProduct(VkPhysicalDevice device);
IntelProduct getIntelProduct(const Vulkan &vulkan);
