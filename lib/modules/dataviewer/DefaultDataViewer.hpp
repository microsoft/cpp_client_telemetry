#pragma once
#include "mat/config.h"

#ifdef HAVE_MAT_DEFAULTDATAVIEWER

#ifndef DEFAULTDATAVIEWER_HPP
#define DEFAULTDATAVIEWER_HPP

#include "public/Version.hpp"
#include "public/IDataViewer.hpp"
#include "public/ctmacros.hpp"

namespace ARIASDK_NS_BEGIN {

    class DefaultDataViewer : public IDataViewer
    {
    public:
        void RecieveData(const std::vector<std::uint8_t>& packetData) noexcept override;

        const char* const GetName() const noexcept override
        {
            return m_name;
        }

    private:
       static constexpr const char* m_name = "DefaultDataViewer";
    };

} ARIASDK_NS_END

#endif

#endif