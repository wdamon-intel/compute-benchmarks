/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "framework/vk/intel_product/get_intel_product_vk.h"
#include "framework/vk/vulkan.h"

#include <iomanip>
#include <volk.h>

namespace std {

inline std::string to_string(VkPhysicalDeviceType physicalDeviceType)
{
    switch (physicalDeviceType)
    {
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:          return std::string("Other");
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return std::string("Integrated GPU");
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:   return std::string("Discrete GPU");
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:    return std::string("Virtual GPU");
        case VK_PHYSICAL_DEVICE_TYPE_CPU:            return std::string("CPU");

        default:
            return std::string("Unknown");
    }
}

} // namespace std

namespace VK {

constexpr uint32_t INTEL_VENDOR_ID = 0x8086;

void printDeviceInfo()
{
    ContextProperties contextProperties = ContextProperties::create().disable();
    QueueProperties queueProperties = QueueProperties::create().disable();
    Vulkan vulkan(queueProperties, contextProperties);

    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(vulkan.instance, &physicalDeviceCount, NULL);

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(vulkan.instance, &physicalDeviceCount, physicalDevices.data());

    for (const auto& physicalDevice : physicalDevices)
    {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

        if (physicalDeviceProperties.vendorID == INTEL_VENDOR_ID)
        {
            IntelProduct intelProduct = getIntelProduct(physicalDeviceProperties.deviceID);
            IntelGen     intelGen     = getIntelGen(intelProduct);

            VkPhysicalDeviceIDProperties physicalDeviceIDProperties;
            physicalDeviceIDProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES;
            physicalDeviceIDProperties.pNext = NULL;

            VkPhysicalDeviceDriverProperties physicalDeviceDriverProperties;
            physicalDeviceDriverProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;
            physicalDeviceDriverProperties.pNext = &physicalDeviceIDProperties;

            VkPhysicalDeviceProperties2 pdp2;
            pdp2.sType      = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
            pdp2.pNext      = &physicalDeviceDriverProperties;
            pdp2.properties = physicalDeviceProperties;

            vkGetPhysicalDeviceProperties2(physicalDevice, &pdp2);

            std::cout << "\tDevice: " << physicalDeviceProperties.deviceName << std::endl;
            std::cout << "\t\tvendorId:     0x" << std::hex << physicalDeviceProperties.vendorID << std::dec << std::endl;
            std::cout << "\t\tdeviceId:     0x" << std::hex << physicalDeviceProperties.deviceID << " (intelProduct=" << std::to_string(intelProduct) << ", intelGen=" << std::to_string(intelGen) << ")" << std::dec << std::endl;
            std::cout << "\t\tdeviceType:   " << std::to_string(physicalDeviceProperties.deviceType) << std::endl;
            std::cout << "\t\tdriverName:   " << physicalDeviceDriverProperties.driverName << std::endl;
            std::cout << "\t\tdriverInfo:   " << physicalDeviceDriverProperties.driverInfo << std::endl;
            std::cout << "\t\tdriverVersion:" << std::to_string((physicalDeviceProperties.driverVersion & 0xFF000000) >> 24) + "." +
                                                 std::to_string((physicalDeviceProperties.driverVersion & 0x00FF0000) >> 16) + "." +
                                                 std::to_string((physicalDeviceProperties.driverVersion & 0x0000FFFF));
        }
    }

    std::cout << std::endl;
}

static void printAvailableDevices()
{
    ContextProperties contextProperties = ContextProperties::create().disable();
    QueueProperties queueProperties = QueueProperties::create().disable();
    Vulkan vulkan(queueProperties, contextProperties);

    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(vulkan.instance, &physicalDeviceCount, NULL);

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(vulkan.instance, &physicalDeviceCount, physicalDevices.data());

    std::cout << "Available Vulkan Physical Devices: " << physicalDeviceCount << std::endl;

    for (uint32_t physicalDeviceIndex = 0; physicalDeviceIndex < physicalDeviceCount; ++physicalDeviceIndex)
    {
        const VkPhysicalDevice physicalDevice = physicalDevices[physicalDeviceIndex];

        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

        std::cout << "Device Type:    "   << std::to_string(physicalDeviceProperties.deviceType) << std::endl;
        std::cout << "Device Name:    "   << physicalDeviceProperties.deviceName << std::endl;
        std::cout << "Vendor ID:      0x" << std::hex << physicalDeviceProperties.vendorID << std::dec << std::endl;
        std::cout << "Device ID:      0x" << std::hex << physicalDeviceProperties.deviceID << std::dec << std::endl;
        std::cout << "Driver Version: 0x" << std::hex << physicalDeviceProperties.driverVersion << std::dec << std::endl;
        std::cout << "                ("  << std::to_string((physicalDeviceProperties.driverVersion & 0xFF000000) >> 24) + "." +
                                             std::to_string((physicalDeviceProperties.driverVersion & 0x00FF0000) >> 16) + "." +
                                             std::to_string((physicalDeviceProperties.driverVersion & 0x0000FFFF)) << ")" << std::endl;
        std::cout << "VK API Version: "   << VK_VERSION_MAJOR(physicalDeviceProperties.apiVersion) << "."
                                          << VK_VERSION_MINOR(physicalDeviceProperties.apiVersion) << "."
                                          << VK_VERSION_PATCH(physicalDeviceProperties.apiVersion);

        if (physicalDeviceProperties.vendorID == INTEL_VENDOR_ID)
        {
            IntelProduct intelProduct = getIntelProduct(physicalDeviceProperties.deviceID);
            IntelGen     intelGen     = getIntelGen(intelProduct);
            std::cout << "Intel Product:  "   << std::to_string(intelProduct) << std::endl;
            std::cout << "Intel Gen:      "   << std::to_string(intelGen) << std::endl;

            std::cout << "Note: select this device with --vkPhysicalDeviceIndex=" << physicalDeviceIndex << std::endl;
        }
    }

    std::cout << std::endl;
}

} // namespace VK
