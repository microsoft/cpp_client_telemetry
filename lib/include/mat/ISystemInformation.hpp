//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef ISYSTEMINFORMATION_HPP
#define ISYSTEMINFORMATION_HPP

#include "pal/PAL.hpp"
#include "IInformationProvider.hpp"

#include <string>

namespace PAL_NS_BEGIN {

    class ISystemInformation : public IInformationProvider
    {
    public:

        virtual ~ISystemInformation() {};

        /// <summary>
        /// Gets the App ID.
        /// </summary>
        /// <returns>The App ID</returns>
        virtual std::string const& GetAppId() const = 0;

        /// <summary>
        /// Gets the App version.
        /// </summary>
        /// <returns>The App version</returns>
        virtual std::string const& GetAppVersion() const = 0;

        /// <summary>
        /// Gets the App language.
        /// </summary>
        virtual std::string const& GetAppLanguage() const = 0;

        /// <summary>
        /// Gets the OS Full version for the current device
        /// </summary>
        /// <returns>The OS full version for the current device</returns>
        virtual std::string const& GetOsFullVersion() const = 0;

        /// <summary>
        /// Gets the OS major version for the current device
        /// </summary>
        /// <returns>The OS major version for the current device</returns>
        virtual std::string const& GetOsMajorVersion() const = 0;

        /// <summary>
        /// The name of the OS.The SDK should ensure this is a limited normalized set. Examples such as "iOS" or "Windows Phone".
        /// </summary>
        /// <returns>The OS Platofrm for the current device</returns>
        virtual std::string const& GetOsName() const = 0;

        /// <summary>
        /// Gets the user language.
        /// </summary>
        /// <returns>The user language</returns>
        virtual std::string const& GetUserLanguage() const = 0;

        /// <summary>
        /// Gets the user time zone.
        /// </summary>
        /// <returns>The user time zone</returns>
        virtual std::string const& GetUserTimeZone() const = 0;

        /// <summary>
        /// Gets the advertising Id, if enabled, for the current user.
        /// </summary>
        /// <returns>Advertising Id</returns>
        virtual std::string const& GetUserAdvertisingId() const = 0;

        /// <summary>
        /// Gets the Device class like desktop, tablet, phone, xbox.
        /// </summary>
        /// <returns>Device class (string)</returns>
        virtual std::string const& GetDeviceClass() const = 0;

        /// <summary>
        /// Gets the Commercial Id
        /// </summary>
        /// <returns>Commercial Id</returns>
        virtual std::string const& GetCommercialId() const = 0;

    };

} PAL_NS_END

#endif

