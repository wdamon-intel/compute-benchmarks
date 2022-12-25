/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "framework/vk/vulkan.h"
#include "framework/vk/utility/error.h"
#include "framework/test_case/register_test_case.h"
#include "framework/utility/timer.h"

#include "definitions/copy_buffer.h"

#include <gtest/gtest.h>

static TestResult run(const CopyBufferArguments &arguments, Statistics &statistics) {
    MeasurementFields typeSelector(MeasurementUnit::GigabytesPerSecond, arguments.useEvents ? MeasurementType::Gpu : MeasurementType::Cpu);

    if (isNoopRun()) {
        statistics.pushUnitAndType(typeSelector.getUnit(), typeSelector.getType());
        return TestResult::Nooped;
    }

    // Setup
    QueueProperties queueProperties = QueueProperties::create().setProfiling(arguments.useEvents);
    Vulkan vulkan(queueProperties);
    if (vulkan.device == nullptr) {
        return TestResult::DeviceNotCapable;
    }

    Timer timer;

    // Create buffers
    VkBufferCreateInfo srcBufCreateInfo;
    srcBufCreateInfo.sType                  = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    srcBufCreateInfo.pNext                  = NULL;
    srcBufCreateInfo.flags                  = 0;
    srcBufCreateInfo.size                   = arguments.size;
    srcBufCreateInfo.usage                  = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    srcBufCreateInfo.sharingMode            = VK_SHARING_MODE_EXCLUSIVE;
    srcBufCreateInfo.queueFamilyIndexCount  = 0;
    srcBufCreateInfo.pQueueFamilyIndices    = NULL;

    VkBufferCreateInfo dstBufCreateInfo;
    dstBufCreateInfo.sType                  = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    dstBufCreateInfo.pNext                  = NULL;
    dstBufCreateInfo.flags                  = 0;
    dstBufCreateInfo.size                   = arguments.size;
    dstBufCreateInfo.usage                  = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    dstBufCreateInfo.sharingMode            = VK_SHARING_MODE_EXCLUSIVE;
    dstBufCreateInfo.queueFamilyIndexCount  = 0;
    dstBufCreateInfo.pQueueFamilyIndices    = NULL;
    
    VkBuffer srcBuf;
    VkBuffer dstBuf;

    ASSERT_VK_SUCCESS(vkCreateBuffer(vulkan.device, &srcBufCreateInfo, NULL, &srcBuf));
    ASSERT_VK_SUCCESS(vkCreateBuffer(vulkan.device, &dstBufCreateInfo, NULL, &dstBuf));

    return TestResult::NoImplementation;
}

static RegisterTestCaseImplementation<CopyBuffer> registerTestCase(run, Api::Vulkan);
