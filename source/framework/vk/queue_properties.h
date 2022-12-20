/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "framework/configuration.h"
#include "framework/enum/device_selection.h"
#include "framework/enum/engine.h"
#include "framework/vk/vk.h"

namespace VK {

struct QueueProperties {
    bool            createQueue                = true;
    bool            requireCreationSuccess     = true;
    bool            profiling                  = false;
    bool            enableValidationLayer      = false;
    bool            enableDebugLayer           = false;
    bool            enableShaderDebugging      = false;

    Engine          selectedEngine             = Engine::Unknown;
    DeviceSelection deviceSelection            = DeviceSelection::Unknown;

    static QueueProperties create() {
        return QueueProperties()
            .setDeviceSelection(Configuration::get().subDeviceSelection);
    }

    QueueProperties &allowCreationFail() {
        requireCreationSuccess = false;
        return *this;
    }

    QueueProperties &setForceBlitter(bool bcs) {
        this->selectedEngine = bcs ? Engine::Bcs : Engine::Unknown;
        return *this;
    }

    QueueProperties &setForceEngine(Engine engine) {
        this->selectedEngine = engine;
        return *this;
    }

    QueueProperties &setProfiling(bool newValue) {
        this->profiling = newValue;
        return *this;
    }

    QueueProperties &disable() {
        createQueue = false;
        return *this;
    }

    QueueProperties &setDeviceSelection(DeviceSelection newValue) {
        FATAL_ERROR_UNLESS(DeviceSelectionHelper::hasSingleDevice(newValue), "Queue can be created only on a single device");
        this->deviceSelection = newValue;
        return *this;
    }

#if 0
    bool fillQueueProperties(cl_device_id device, cl_queue_properties properties[], size_t size) const {
        std::fill_n(properties, size, 0);
        properties[0] = CL_QUEUE_PROPERTIES;
        cl_int propertiesIndex = 2;
        if (this->profiling) {
            properties[1] |= CL_QUEUE_PROFILING_ENABLE;
        }
        if (this->ooq == 1 || (this->ooq == -1 && Configuration::get().oclUseOOQ)) {
            properties[1] |= CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
        }
        if (this->selectedEngine != Engine::Unknown) {
            const auto propertiesForBlitter = QueueFamiliesHelper::getPropertiesForSelectingEngine(device, this->selectedEngine);
            if (propertiesForBlitter == nullptr) {
                return false;
            }

            for (auto i = 0u; i < propertiesForBlitter->propertiesCount; i++) {
                properties[propertiesIndex++] = propertiesForBlitter->properties[i];
            }
        }
        return true;
    }
#endif // 0
};

} // namespace VK
