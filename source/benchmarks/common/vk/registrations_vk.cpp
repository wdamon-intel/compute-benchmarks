/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "framework/print_device_info.h"
#include "framework/supported_apis.h"
#include "framework/vk/vk.h"
#include "framework/vk/utility/print_device_info_vk.inl"
#include "framework/utility/execute_at_app_init.h"

EXECUTE_AT_APP_INIT {
    DeviceInfo::registerFunctions(Api::Vulkan, VK::printDeviceInfo, VK::printAvailableDevices);
    SupportedApis::registerSupportedApi(Api::Vulkan);
};
