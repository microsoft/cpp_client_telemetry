//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef IDEVICEINFORMATION_HPP
#define IDEVICEINFORMATION_HPP

#include "pal/PAL.hpp"

#include "Enums.hpp"
#include "IInformationProvider.hpp"

#include <string>

// Property Name
#define STORAGE_SIZE "StorageSize"
#define POWER_SOURCE "PowerSource"

using namespace MAT;

namespace PAL_NS_BEGIN {

    class IDeviceInformation : public IInformationProvider
    {
    public:

        virtual ~IDeviceInformation() {};

        /// <summary>
        /// Gets the unique ID of the current device
        /// </summary>
        /// <returns>The unique ID of the current device</returns>
        virtual std::string const& GetDeviceId() const = 0;

        /// <summary>
        /// Gets the manufacturer of the current device
        /// </summary>
        /// <returns>The manufacturer of the current device</returns>
        virtual std::string const& GetManufacturer() const = 0;

        /// <summary>
        /// Gets the model of the current device
        /// </summary>
        /// <returns>The model of the current device</returns>
        virtual std::string const& GetModel() const = 0;

        /// <summary>
        /// The OS achitecture type, such as "x86" or "x64".
        /// </summary>
        /// <returns>OS architecture</returns>
        virtual OsArchitectureType GetOsArchitectureType() const = 0;

        /// <summary>
        /// Gets the power source the device is currently using.
        /// </summary>
        /// <returns>Source of power the device is using</returns>
        virtual PowerSource GetPowerSource() const = 0;

        /// <summary>
        /// Gets the device ticket.
        /// </summary>
        /// <returns>Device ticket</returns>
        virtual std::string GetDeviceTicket() const = 0;
    };

} PAL_NS_END

#endif

