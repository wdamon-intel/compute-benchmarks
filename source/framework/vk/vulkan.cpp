/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "vulkan.h"

#define VOLK_IMPLEMENTATION
#include <volk.h>

#include "framework/vk/utility/error.h"

#include <vector>

namespace VK {

static void initializeVolk()
{
    static bool volkInitialized __attribute__((used)) = [](){
        volkInitialize();
        return true;
    }();
}

Vulkan::Vulkan(const QueueProperties &queueProperties, const ContextProperties &contextProperties)
{
    initializeVolk();

    (void)queueProperties;      // unused parameter
    (void)contextProperties;    // unused parameter

    {
        constexpr size_t MAX_ENABLED_EXTENSIONS = 64;

        uint32_t instanceExtensionPropertyCount = 0;
        EXPECT_VK_SUCCESS(vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionPropertyCount, NULL));

        std::vector<VkExtensionProperties> instanceExtensionProperties(instanceExtensionPropertyCount);
        EXPECT_VK_SUCCESS(vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionPropertyCount, instanceExtensionProperties.data()));

        std::vector<const char*> enabledExtensionNames;
        enabledExtensionNames.reserve(MAX_ENABLED_EXTENSIONS);
        {
            VkBool32 bSurfaceExtensionFound         = VK_FALSE;
            VkBool32 bPlatformSurfaceExtensionFound = VK_FALSE;

            for (const auto& i : instanceExtensionProperties)
            {
                if (!strncmp(VK_KHR_SURFACE_EXTENSION_NAME, i.extensionName, VK_MAX_EXTENSION_NAME_SIZE))
                {
                    bSurfaceExtensionFound = VK_TRUE;
                    enabledExtensionNames.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
                }
#if defined(VK_USE_PLATFORM_WIN32_KHR)
                if (!strncmp(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, i.extensionName, VK_MAX_EXTENSION_NAME_SIZE))
                {
                    bPlatformSurfaceExtensionFound = VK_TRUE;
                    enabledExtensionNames.emplace_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
                }
#elif defined(VK_USE_PLATFORM_XCB_KHR)
                if (!strncmp(VK_KHR_XCB_SURFACE_EXTENSION_NAME, i.extensionName, VK_MAX_EXTENSION_NAME_SIZE))
                {
                    bPlatformSurfaceExtensionFound = VK_TRUE;
                    enabledExtensionNames.emplace_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
                }
#else
                #error "VK_USE_PLATFORM_XXX must be defined."
#endif
            }

            if (!bSurfaceExtensionFound)
            {
                FATAL_ERROR("vkEnumerateInstanceExtensionProperties failed to find the " VK_KHR_SURFACE_EXTENSION_NAME " extension.\n"
                            "Do you have a compatible Vulkan installable client driver (ICD) installed?");
            }
#if defined(VK_USE_PLATFORM_WIN32_KHR)
            if (!bPlatformSurfaceExtensionFound)
            {
                FATAL_ERROR("vkEnumerateInstanceExtensionProperties failed to find the " VK_KHR_WIN32_SURFACE_EXTENSION_NAME " extension.\n"
                            "Do you have a compatible Vulkan installable client driver (ICD) installed?");
            }
#elif defined(VK_USE_PLATFORM_XCB_KHR)
            if (!bPlatformSurfaceExtensionFound)
            {
                FATAL_ERROR("vkEnumerateInstanceExtensionProperties failed to find the " VK_KHR_XCB_SURFACE_EXTENSION_NAME " extension.\n"
                            "Do you have a compatible Vulkan installable client driver (ICD) installed?");
            }
#endif
            enabledExtensionNames.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        }

        VkApplicationInfo appInfo;
        appInfo.sType                       = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pNext                       = NULL;
        appInfo.pApplicationName            = NULL;
        appInfo.applicationVersion          = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName                 = NULL;
        appInfo.engineVersion               = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion                  = VK_API_VERSION_1_3; 

        VkInstanceCreateInfo createInfo;
        createInfo.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pNext                    = NULL;
        createInfo.flags                    = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        createInfo.pApplicationInfo         = &appInfo;
        createInfo.enabledLayerCount        = 0;
        createInfo.ppEnabledLayerNames      = NULL;
        createInfo.enabledExtensionCount    = enabledExtensionNames.size();
        createInfo.ppEnabledExtensionNames  = enabledExtensionNames.data();

        VK_SUCCESS_OR_ERROR(vkCreateInstance(&createInfo, NULL, &instance), "vkCreateInstance: ");

        volkLoadInstance(instance);
    }
}

Vulkan::~Vulkan()
{
    if (instance != nullptr)
    {
        vkDestroyInstance(instance, NULL);
        instance = nullptr;
    }
}

} // namespace VK 
