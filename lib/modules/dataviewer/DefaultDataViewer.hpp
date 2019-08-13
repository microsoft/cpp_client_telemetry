#pragma once
#include "mat/config.h"

#ifndef DEFAULTDATAVIEWER_HPP
#define DEFAULTDATAVIEWER_HPP

#include "public/Version.hpp"
#include "public/IDataViewer.hpp"
#include "public/ctmacros.hpp"

namespace ARIASDK_NS_BEGIN {

    class MATSDK_LIBABI DefaultDataViewer final : public IDataViewer
    {
    public:
        void RecieveData(const std::vector<std::uint8_t>& packetData) noexcept override;

        const char* const GetName() const noexcept override
        {
            return m_name;
        }

        bool EnableRemoteViewer(const char * endpoint);
        bool EnableLocalViewer();
        bool EnableLocalViewer(const std::string& AppId, const std::string& AppPackage);
        bool DisableViewer();

    private:
       static constexpr const char* m_name = "DefaultDataViewer";
    };

} ARIASDK_NS_END

#endif