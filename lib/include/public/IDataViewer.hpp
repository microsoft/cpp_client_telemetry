// Copyright (c) Microsoft. All rights reserved.
#ifndef IDATAVIEWER_HPP
#define IDATAVIEWER_HPP

#include "Version.hpp"
#include <vector>

namespace ARIASDK_NS_BEGIN
{
    /// <summary>
    /// This interface allows SDK users to register a data viewer
    /// that will receive all packets uploaded by the SDK.
    /// </summary>
    class IDataViewer
    {
    public:
        /// <summary>
        /// This method allows SDK to pass the uploaded packet to the data viewer.
        /// </summary>
        /// <param name="packetData">HTTP Request Packet as a binary blob.</param>
        virtual void RecieveData(const std::vector<std::uint8_t>& packetData) const noexcept = 0;

        /// <summary>
        /// Get the name of the current viewer.
        /// </summary>
        virtual const char* const GetName() const noexcept = 0;
    };

} ARIASDK_NS_END

#endif
