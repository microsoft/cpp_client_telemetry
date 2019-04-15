// Copyright (c) Microsoft. All rights reserved.
#include <string.h>

#include "LogSessionData.hpp"
#include "pal/PAL.hpp"
#include "Logger.hpp"
#include "DebugEvents.hpp"

#include "utils/Utils.hpp"
#include "utils/StringUtils.hpp"
#include "utils/FileUtils.hpp"

#include <memory>

using namespace std;

namespace ARIASDK_NS_BEGIN {

    inline void remove_eol(string& result)
    {
        if (!result.empty() && result[result.length() - 1] == '\n')
        {
            result.erase(result.length() - 1);
        }
    }

    LogSessionData::LogSessionData(string const& cacheFilePath)
    {
        m_sessionFirstTimeLaunch = PAL::getUtcSystemTimeMs();
        string sessionPath = cacheFilePath.empty() ? "" : (cacheFilePath + ".ses").c_str();
        if (!sessionPath.empty())
        {
            open(sessionPath);
        }
    }

    void LogSessionData::open(const string& path)
    {
        bool recreate = true;

        if (MAT::FileExists(path.c_str()))
        {
            auto contents = MAT::FileGetContents(path.c_str());
            if (!contents.empty())
            {
                vector<string> v;
                StringUtils::SplitString(contents, '\n', v);
                if (v.size() == 2)
                {
                    remove_eol(v[0]);
                    remove_eol(v[1]);
                    try
                    {
                       // First launch time
                       m_sessionFirstTimeLaunch = std::stoull(v[0]);
                       // SDK UUID
                       m_sessionSDKUid = v[1];
                       recreate = false;
                    }
                    catch (const std::invalid_argument& invalidArgumentException)
                    {
                       LOG_ERROR("Non-integer data passed to std::stoull");
                       recreate = true;
                    }
                    catch (const std::out_of_range& outOfRangeException)
                    {
                       LOG_ERROR("Value passed to std::stoull was larger than unsinged long long could represent");
                       recreate = true;
                    }
                }
            }
        }

        if (recreate)
        {
            m_sessionFirstTimeLaunch = PAL::getUtcSystemTimeMs();
            m_sessionSDKUid = PAL::generateUuidString();
            string contents;
            contents += std::to_string(m_sessionFirstTimeLaunch);
            contents += '\n';
            contents += m_sessionSDKUid;
            contents += '\n';
            if (!MAT::FileWrite(path.c_str(), contents.c_str()))
            {
                LOG_WARN("Unable to save session analytics to %s", path.c_str());
            }
        }

    }

} ARIASDK_NS_END

