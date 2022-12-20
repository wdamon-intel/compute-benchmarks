/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "framework/vk/utility/error_codes.h"
#include "framework/utility/error.h"

#include <string>

#define ASSERT_VK_SUCCESS(retVal)                                                                                                           \
    {                                                                                                                                       \
        const VkResult tempVarForDefine = (retVal);                                                                                         \
        if (tempVarForDefine != VK_SUCCESS) {                                                                                               \
            NON_FATAL_ERROR("ASSERT_VK_SUCCESS", #retVal, std::to_string(tempVarForDefine).c_str(), vkErrorToString(tempVarForDefine));     \
            return TestResult::Error;                                                                                                       \
        }                                                                                                                                   \
    }                                                                                                                                       \

#define EXPECT_VK_SUCCESS(retVal)                                                                                                           \
    {                                                                                                                                       \
        const VkResult tempVarForDefine = (retVal);                                                                                         \
        if (tempVarForDefine != VK_SUCCESS) {                                                                                               \
            NON_FATAL_ERROR("EXPECT_VK_SUCCESS", #retVal, std::to_string(tempVarForDefine).c_str(), vkErrorToString(tempVarForDefine));     \
        }                                                                                                                                   \
    }                                                                                                                                       \

#define VK_SUCCESS_OR_RETURN_VALUE(retVal, value)                                                                                           \
    if ((retVal) != VK_SUCCESS) {                                                                                                           \
        return (value);                                                                                                                     \
    }                                                                                                                                       \

#define VK_SUCCESS_OR_RETURN_FALSE(retVal)                                                                                                  \
    VK_SUCCESS_OR_RETURN_VALUE((retVal), false)

#define VK_SUCCESS_OR_RETURN_ERROR(retVal)                                                                                                  \
    VK_SUCCESS_OR_RETURN_VALUE((retVal), TestResult::Error)

#define VK_SUCCESS_OR_RETURN(retVal)                                                                                                        \
    {                                                                                                                                       \
        const VkResult tempVarForDefine = (retVal);                                                                                         \
        VK_SUCCESS_OR_RETURN_VALUE(tempVarForDefine, tempVarForDefine);                                                                     \
    }                                                                                                                                       \

#define VK_SUCCESS_OR_ERROR(retVal, message)                                                                                                \
    {                                                                                                                                       \
        const VkResult tempVarForDefine = (retVal);                                                                                         \
        if (tempVarForDefine != VK_SUCCESS) {                                                                                               \
            const auto _message = std::string(message) + " (Fatal Vulkan error occurred, retVal=" + std::to_string(tempVarForDefine) + ")"; \
            FATAL_ERROR(_message);                                                                                                          \
        }                                                                                                                                   \
    }                                                                                                                                       \

