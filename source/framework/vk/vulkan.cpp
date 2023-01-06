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
#include "framework/vk/utility/vk_command_pool.h"

#if VULKAN_PRINT_QUEUE_PROPERTIES
#include <string>
#include <iostream>
#include <sstream>
#endif // VULKAN_PRINT_QUEUE_PROPERTIES

#define UNUSED_PARAMETER(X)                     (void)X;

#if VULKAN_PRINT_QUEUE_PROPERTIES
static std::string vkQueueFlagsToString(const VkQueueFlags& queueFlags)
{
    const uint32_t numFlags = __builtin_popcount(queueFlags);
    if (numFlags == 0) {
        return std::string("NONE");
    } else {
        std::vector<std::string> names;
        names.reserve(numFlags);
        if (queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            names.emplace_back(std::string("VK_QUEUE_GRAPHICS_BIT"));
        }
        if (queueFlags & VK_QUEUE_COMPUTE_BIT) {
            names.emplace_back(std::string("VK_QUEUE_COMPUTE_BIT"));
        }
        if (queueFlags & VK_QUEUE_TRANSFER_BIT) {
            names.emplace_back(std::string("VK_QUEUE_TRANSFER_BIT"));
        }
        if (queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
            names.emplace_back(std::string("VK_QUEUE_SPARSE_BINDING_BIT"));
        }
        if (queueFlags & VK_QUEUE_PROTECTED_BIT) {
            names.emplace_back(std::string("VK_QUEUE_PROTECTED_BIT"));
        }
        std::stringstream ss;
        ss << names[0];
        for (size_t index = 1; index < names.size(); ++index) {
            ss << " | " << names[index];
        }
        return ss.str();
    }
}
#endif // VULKAN_PRINT_QUEUE_PROPERTIES

namespace VK {

static void initializeVolk()
{
    [[maybe_unused]]
    static bool volkInitialized = [](){
        volkInitialize();
        return true;
    }();
}

void Vulkan::_selectInstanceLayers(const QueueProperties &queueProperties, const ContextProperties &contextProperties)
{
#define VALIDATION_LAYER_NAME "VK_LAYER_KHRONOS_validation"

    UNUSED_PARAMETER(contextProperties);

    if (queueProperties.enableValidationLayer)
    {
        uint32_t propertyCount = 0;
        EXPECT_VK_SUCCESS(vkEnumerateInstanceLayerProperties(&propertyCount, nullptr));

        std::vector<VkLayerProperties> properties(propertyCount);
        EXPECT_VK_SUCCESS(vkEnumerateInstanceLayerProperties(&propertyCount, properties.data()));

        VkBool32 bValidationLayerPropertyFound = VK_FALSE;
        for (const auto& property : properties)
        {
            if (!strncmp(VALIDATION_LAYER_NAME, property.layerName, VK_MAX_EXTENSION_NAME_SIZE))
            {
                bValidationLayerPropertyFound = VK_TRUE;
                enabledInstanceLayers.push_back(VALIDATION_LAYER_NAME);
            }
        }

        FATAL_ERROR_IF(!bValidationLayerPropertyFound, "vkEnumerateInstanceLayerProperties failed to find the " VALIDATION_LAYER_NAME " layer.\n"
                    "Do you have a compatible Vulkan installable client driver (ICD) installed?");
    }

#undef VALIDATION_LAYER_NAME
}

void Vulkan::_selectInstanceExtensions(const QueueProperties &queueProperties, const ContextProperties &contextProperties)
{
    UNUSED_PARAMETER(queueProperties);
    UNUSED_PARAMETER(contextProperties);

    uint32_t propertyCount = 0;
    EXPECT_VK_SUCCESS(vkEnumerateInstanceExtensionProperties(NULL, &propertyCount, NULL));

    std::vector<VkExtensionProperties> properties(propertyCount);
    EXPECT_VK_SUCCESS(vkEnumerateInstanceExtensionProperties(NULL, &propertyCount, properties.data()));

    enabledInstanceExtensions.reserve(static_cast<size_t>(Limits::MAX_ENABLED_EXTENSIONS));
    {
        VkBool32 bSurfaceExtensionFound         = VK_FALSE;
        VkBool32 bPlatformSurfaceExtensionFound = VK_FALSE;

        for (const auto& property : properties)
        {
            if (!strncmp(VK_KHR_SURFACE_EXTENSION_NAME, property.extensionName, VK_MAX_EXTENSION_NAME_SIZE))
            {
                bSurfaceExtensionFound = VK_TRUE;
                enabledInstanceExtensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
            }
#if defined(VK_USE_PLATFORM_WIN32_KHR)
            if (!strncmp(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, property.extensionName, VK_MAX_EXTENSION_NAME_SIZE))
            {
                bPlatformSurfaceExtensionFound = VK_TRUE;
                enabledInstanceExtensions.emplace_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
            }
#elif defined(VK_USE_PLATFORM_XCB_KHR)
            if (!strncmp(VK_KHR_XCB_SURFACE_EXTENSION_NAME, property.extensionName, VK_MAX_EXTENSION_NAME_SIZE))
            {
                bPlatformSurfaceExtensionFound = VK_TRUE;
                enabledInstanceExtensions.emplace_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
            }
#else
            #error "VK_USE_PLATFORM_XXX must be defined."
#endif
            if (!strncmp(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, property.extensionName, VK_MAX_EXTENSION_NAME_SIZE))
            {
                enabledInstanceExtensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
            }

            if (!strncmp(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME, property.extensionName, VK_MAX_EXTENSION_NAME_SIZE))
            {
                enabledInstanceExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            }
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
    }
}

void Vulkan::_createInstance(const QueueProperties &queueProperties, const ContextProperties &contextProperties)
{
    _selectInstanceLayers(queueProperties, contextProperties);
    _selectInstanceExtensions(queueProperties, contextProperties);

    VkApplicationInfo appInfo;
    appInfo.sType                               = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext                               = NULL;
    appInfo.pApplicationName                    = NULL;
    appInfo.applicationVersion                  = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName                         = NULL;
    appInfo.engineVersion                       = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion                          = VK_API_VERSION_1_3; 

    VkInstanceCreateInfo instanceCreateInfo;
    instanceCreateInfo.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext                    = NULL;
    instanceCreateInfo.flags                    = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    instanceCreateInfo.pApplicationInfo         = &appInfo;
    instanceCreateInfo.enabledLayerCount        = static_cast<uint32_t>(enabledInstanceLayers.size());
    instanceCreateInfo.ppEnabledLayerNames      = enabledInstanceLayers.data();
    instanceCreateInfo.enabledExtensionCount    = static_cast<uint32_t>(enabledInstanceExtensions.size());
    instanceCreateInfo.ppEnabledExtensionNames  = enabledInstanceExtensions.data();

    VK_SUCCESS_OR_ERROR(vkCreateInstance(&instanceCreateInfo, NULL, &instance), "vkCreateInstance: ");

    volkLoadInstanceOnly(instance);
}

void Vulkan::_selectPhysicalDevice(const QueueProperties &queueProperties, const ContextProperties &contextProperties)
{
    UNUSED_PARAMETER(queueProperties);
    UNUSED_PARAMETER(contextProperties);

    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, NULL);

    const auto requestedPhysicalDeviceIndex = Configuration::get().vkPhysicalDeviceIndex;
    if (requestedPhysicalDeviceIndex >= physicalDeviceCount)
    {
        FATAL_ERROR("Invalid VK physical device index. physicalDeviceIndex=", requestedPhysicalDeviceIndex, " numPhysicalDevices=", physicalDeviceCount);
    }

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

    physicalDevice = physicalDevices[requestedPhysicalDeviceIndex];
}

void Vulkan::_selectDeviceExtensions(const QueueProperties &queueProperties, const ContextProperties &contextProperties)
{
    UNUSED_PARAMETER(contextProperties);

    uint32_t propertyCount = 0;
    EXPECT_VK_SUCCESS(vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &propertyCount, NULL));

    std::vector<VkExtensionProperties> properties(propertyCount);
    EXPECT_VK_SUCCESS(vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &propertyCount, properties.data()));

    enabledDeviceExtensions.reserve(static_cast<size_t>(Limits::MAX_ENABLED_EXTENSIONS));
    {
        VkBool32 bSwapchainExtensionFound           = VK_FALSE;
        VkBool32 bIncrementalPresentExtensionFound  = VK_FALSE;

        for (const auto& property : properties)
        {
            if (!strncmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME, property.extensionName, VK_MAX_EXTENSION_NAME_SIZE))
            {
                bSwapchainExtensionFound = VK_TRUE;
                enabledDeviceExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
            }

            if (!strncmp("VK_KHR_portability_subset", property.extensionName, VK_MAX_EXTENSION_NAME_SIZE))
            {
                enabledDeviceExtensions.emplace_back("VK_KHR_portability_subset");
            }

            if (queueProperties.enableIncrementalPresent)
            {
                if (!strncmp(VK_KHR_INCREMENTAL_PRESENT_EXTENSION_NAME, property.extensionName, VK_MAX_EXTENSION_NAME_SIZE))
                {
                    bIncrementalPresentExtensionFound = VK_TRUE;
                    enabledDeviceExtensions.emplace_back(VK_KHR_INCREMENTAL_PRESENT_EXTENSION_NAME);
                }
            }
        }

        if (queueProperties.enableIncrementalPresent && !bIncrementalPresentExtensionFound)
        {
            const VkResult result = VK_ERROR_EXTENSION_NOT_PRESENT;

            std::string message(vkErrorToString(result));
            message += ": " VK_KHR_INCREMENTAL_PRESENT_EXTENSION_NAME;

            NON_FATAL_ERROR("EXPECT_VK_SUCCESS", "VK_ERROR_EXTENSION_NOT_PRESENT", std::to_string(result).c_str(), message.c_str());
        }

        if (!bSwapchainExtensionFound)
        {
            const VkResult result = VK_ERROR_EXTENSION_NOT_PRESENT;

            std::string message(vkErrorToString(result));
            message += ": " VK_KHR_SWAPCHAIN_EXTENSION_NAME;

            NON_FATAL_ERROR("EXPECT_VK_SUCCESS", "VK_ERROR_EXTENSION_NOT_PRESENT", std::to_string(result).c_str(), message.c_str());
        }
    }
}

void Vulkan::_selectQueueFamilies(const QueueProperties &queueProperties, const ContextProperties &contextProperties)
{
    UNUSED_PARAMETER(queueProperties);
    UNUSED_PARAMETER(contextProperties);

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);

    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

    for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount; ++queueFamilyIndex)
    {
#if VULKAN_PRINT_QUEUE_PROPERTIES
        std::cout << vkQueueFlagsToString(queueFamilyProperties[queueFamilyIndex].queueFlags) << std::endl;
#endif // VULKAN_PRINT_QUEUE_PROPERTIES

        //
        // @todo: select graphics, compute, transfer and present queue family index values.
        //
    }
}

void Vulkan::_createDevice(const QueueProperties &queueProperties, const ContextProperties &contextProperties)
{
    _selectPhysicalDevice(queueProperties, contextProperties);
    _selectDeviceExtensions(queueProperties, contextProperties);
    _selectQueueFamilies(queueProperties, contextProperties);

    const float queuePriorities[1] = { 0.0f };

    VkDeviceQueueCreateInfo deviceQueueCreateInfo;
    deviceQueueCreateInfo.sType                 = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo.pNext                 = NULL;
    deviceQueueCreateInfo.flags                 = 0;
    deviceQueueCreateInfo.queueFamilyIndex      = 0;
    deviceQueueCreateInfo.queueCount            = 1;
    deviceQueueCreateInfo.pQueuePriorities      = queuePriorities;

    uint32_t deviceExtensionCount = 0;
    EXPECT_VK_SUCCESS(vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &deviceExtensionCount, NULL));

    std::vector<VkExtensionProperties> deviceExtensions(deviceExtensionCount);
    EXPECT_VK_SUCCESS(vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &deviceExtensionCount, deviceExtensions.data()));

    VkDeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.sType                      = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext                      = NULL;
    deviceCreateInfo.flags                      = 0;
    deviceCreateInfo.queueCreateInfoCount       = 1;
    deviceCreateInfo.pQueueCreateInfos          = &deviceQueueCreateInfo;
    deviceCreateInfo.enabledLayerCount          = 0;
    deviceCreateInfo.ppEnabledLayerNames        = NULL;
    deviceCreateInfo.enabledExtensionCount      = static_cast<uint32_t>(enabledDeviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames    = enabledDeviceExtensions.data();
    deviceCreateInfo.pEnabledFeatures           = NULL;

    VK_SUCCESS_OR_ERROR(vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device), "vkCreateDevice: ");

    volkLoadDevice(device);
}

Vulkan::Vulkan(const QueueProperties &queueProperties, const ContextProperties &contextProperties)
{
    initializeVolk();

    _createInstance(queueProperties, contextProperties);
    _createDevice(queueProperties, contextProperties);

    vkGetDeviceQueue(device, 0, 0, &_queues[static_cast<uint32_t>(QueueIndex::GRAPHICS)]);

    _commandPool = std::make_unique<CommandPool>();
    VK_SUCCESS_OR_ERROR(_commandPool->init(device, graphicsQueue(), 3, 0, VK_COMMAND_BUFFER_LEVEL_PRIMARY), "VK::CommandPool::init: ");
}

Vulkan::~Vulkan()
{
    _commandPool.reset();

    if (device != nullptr)
    {
        vkDestroyDevice(device, NULL);
        device = nullptr;
    }
    if (instance != nullptr)
    {
        vkDestroyInstance(instance, NULL);
        instance = nullptr;
    }
}

VkCommandBuffer Vulkan::commandBuffer(uint32_t *pIndex)
{
    static uint32_t index = 0;

    if (VK_SUCCESS != _commandPool->beginPrimary(index, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)) {
        VK_SUCCESS_OR_ERROR(_commandPool->flush(index, true), "could not auto-flush command buffer");
        VK_SUCCESS_OR_ERROR(_commandPool->beginPrimary(index, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT), "could not start command buffer");
    }

    *pIndex = index;

    index = (index + 1) % _commandPool->numCommandBuffers();

    return _commandPool->commandBufferAt(*pIndex);
}

void Vulkan::endEncoding(uint32_t index, EndEncodingFlags flags)
{
    switch (flags)
    {
        case EndEncodingFlags::NONE:
            // fallthrough

        case EndEncodingFlags::FLUSH:
            _commandPool->flush(index);
            break;

        case EndEncodingFlags::FINISH:
            _commandPool->flush(index, true);
            break;
    }
}

} // namespace VK 
