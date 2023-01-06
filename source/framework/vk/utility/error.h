/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "framework/utility/error.h"
#include "framework/vk/utility/error_codes.h"

#include <string>

#define ASSERT_VK_SUCCESS(retVal)                                                                                                       \
    {                                                                                                                                   \
        const VkResult tempVarForDefine = (retVal);                                                                                     \
        if (tempVarForDefine != VK_SUCCESS) {                                                                                           \
            NON_FATAL_ERROR("ASSERT_VK_SUCCESS", #retVal, std::to_string(tempVarForDefine).c_str(), vkErrorToString(tempVarForDefine)); \
            return TestResult::Error;                                                                                                   \
        }                                                                                                                               \
    }

#define EXPECT_VK_SUCCESS(retVal)                                                                                                       \
    {                                                                                                                                   \
        const VkResult tempVarForDefine = (retVal);                                                                                     \
        if (tempVarForDefine != VK_SUCCESS) {                                                                                           \
            NON_FATAL_ERROR("EXPECT_VK_SUCCESS", #retVal, std::to_string(tempVarForDefine).c_str(), vkErrorToString(tempVarForDefine)); \
        }                                                                                                                               \
    }

#define VK_SUCCESS_OR_RETURN_VALUE(retVal, value)                                                                                                \
    {                                                                                                                                            \
        const VkResult tempVarForDefine = (retVal);                                                                                              \
        if (tempVarForDefine != VK_SUCCESS) {                                                                                                    \
            NON_FATAL_ERROR("VK_SUCCESS_OR_RETURN_VALUE", #retVal, std::to_string(tempVarForDefine).c_str(), vkErrorToString(tempVarForDefine)); \
            return (value);                                                                                                                      \
        }                                                                                                                                        \
    }

#define VK_CHECK_PARAMETER_OR_RETURN(_condition, _message)                                                \
    if (!(_condition)) {                                                                                  \
        const VkResult retVal = VK_INCOMPLETE;                                                            \
        NON_FATAL_ERROR("VK_CHECK_PARAMETER", "VK_INCOMPLETE", std::to_string(retVal).c_str(), _message); \
        return retVal;                                                                                    \
    }

#define VK_CHECK_PARAMETER_OR_RETURN_NULL(_condition, _message)                                           \
    if (!(_condition)) {                                                                                  \
        const VkResult retVal = VK_INCOMPLETE;                                                            \
        NON_FATAL_ERROR("VK_CHECK_PARAMETER", "VK_INCOMPLETE", std::to_string(retVal).c_str(), _message); \
        return NULL;                                                                                      \
    }

#define VK_SUCCESS_OR_RETURN_FALSE(retVal) \
    VK_SUCCESS_OR_RETURN_VALUE((retVal), false)

#define VK_SUCCESS_OR_RETURN_ERROR(retVal) \
    VK_SUCCESS_OR_RETURN_VALUE((retVal), TestResult::Error)

#define VK_SUCCESS_OR_RETURN(retVal)                                                                                                       \
    {                                                                                                                                      \
        const VkResult tempVarForDefine = (retVal);                                                                                        \
        if (tempVarForDefine != VK_SUCCESS) {                                                                                              \
            NON_FATAL_ERROR("VK_SUCCESS_OR_RETURN", #retVal, std::to_string(tempVarForDefine).c_str(), vkErrorToString(tempVarForDefine)); \
            return tempVarForDefine;                                                                                                       \
        }                                                                                                                                  \
    }

#define VK_SUCCESS_OR_ERROR(retVal, message)                                                                                                \
    {                                                                                                                                       \
        const VkResult tempVarForDefine = (retVal);                                                                                         \
        if (tempVarForDefine != VK_SUCCESS) {                                                                                               \
            const auto _message = std::string(message) + " (Fatal Vulkan error occurred, retVal=" + std::to_string(tempVarForDefine) + ")"; \
            FATAL_ERROR(_message);                                                                                                          \
        }                                                                                                                                   \
    }
