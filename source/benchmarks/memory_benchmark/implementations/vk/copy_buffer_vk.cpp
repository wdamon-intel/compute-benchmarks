/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "framework/test_case/register_test_case.h"
#include "framework/utility/timer.h"
#include "framework/vk/utility/error.h"
#include "framework/vk/utility/vk_buffer.h"
#include "framework/vk/vulkan.h"

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
    VK::Buffer srcBuffer(&vulkan, arguments.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK::Buffer dstBuffer(&vulkan, arguments.size,                                    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // Warmup
    srcBuffer.fill(arguments.contents);
    dstBuffer.fill(arguments.contents);

    // Benchmark
    for (auto i = 0u; i < arguments.iterations; ++i) {
        timer.measureStart();
        dstBuffer.copyFrom(srcBuffer, 0, 0, arguments.size, true);
        timer.measureEnd();

        statistics.pushValue(timer.get(), arguments.size, typeSelector.getUnit(), typeSelector.getType());
    }

    return TestResult::Success;
}

static RegisterTestCaseImplementation<CopyBuffer> registerTestCase(run, Api::Vulkan);
