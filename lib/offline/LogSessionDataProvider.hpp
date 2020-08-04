#ifndef MAT_LOGSESSIONDATA_PROVIDER_HPP
#define MAT_LOGSESSIONDATA_PROVIDER_HPP

#include "LogSessionData.hpp"
#include "utils/FileUtils.hpp"
#include "utils/StringUtils.hpp"

#include <memory>
namespace ARIASDK_NS_BEGIN
{
    static const char* sessionFirstLaunchTimeName = "sessionfirstlaunchtime";
    static const char* sessionSdkUidName = "sessionsdkuid";

    class  LogSessionDataProvider
    {
    public:
        static std::shared_ptr<LogSessionData> CreateLogSessionData(
            IOfflineStorage* offlineStorage)
        {
            std::string sessionSDKUid;
            unsigned long long sessionFirstTimeLaunch = 0;
            sessionSDKUid = offlineStorage->GetSetting(sessionSdkUidName);
            sessionFirstTimeLaunch = convertStrToLong(offlineStorage->GetSetting(sessionFirstLaunchTimeName));
            if (!sessionFirstTimeLaunch || sessionSDKUid.empty()) {
                sessionFirstTimeLaunch = PAL::getUtcSystemTimeMs();
                sessionSDKUid = PAL::generateUuidString();
                if (!offlineStorage->StoreSetting(sessionFirstLaunchTimeName, std::to_string(sessionFirstTimeLaunch))) {
                    LOG_WARN("Unable to save session analytics to DB for %s", sessionFirstLaunchTimeName);
                }
                if (!offlineStorage->StoreSetting(sessionSdkUidName, sessionSDKUid)) {
                    LOG_WARN("Unable to save session analytics to DB for %s", sessionSDKUid);
                }
            }
            return std::make_shared<LogSessionData>(sessionFirstTimeLaunch, sessionSDKUid);
        }

        static std::shared_ptr<LogSessionData> CreateLogSessionData(
            std::string const& cacheFilePath)
        {
            std::string sessionSDKUid;
            unsigned long long sessionFirstTimeLaunch = 0;
            std::string sessionPath = cacheFilePath.empty() ? "" : (cacheFilePath + ".ses").c_str();
            if (!sessionPath.empty()) {
                if (MAT::FileExists(sessionPath.c_str())) {
                    auto content = MAT::FileGetContents(sessionPath.c_str());
                    if (!parse (content, sessionFirstTimeLaunch, sessionSDKUid)) {
                        sessionFirstTimeLaunch = PAL::getUtcSystemTimeMs();
                        sessionSDKUid = PAL::generateUuidString();
                        writeFileContents(sessionPath, sessionFirstTimeLaunch, sessionSDKUid);
                    }
                } else {
                    sessionFirstTimeLaunch = PAL::getUtcSystemTimeMs();
                    sessionSDKUid = PAL::generateUuidString();
                    writeFileContents(sessionPath, sessionFirstTimeLaunch, sessionSDKUid);
                }
            }
            return std::make_shared<LogSessionData>(sessionFirstTimeLaunch, sessionSDKUid);
        }
    private:
        static unsigned long long convertStrToLong(const std::string& s)
        {
            unsigned long long res = 0ull;
            try
            {
                res = std::stoull(s);
            }
            catch (const std::invalid_argument&)
            {
                LOG_WARN("Non-integer data passed to std::stoull");
            }
            catch (const std::out_of_range&)
            {
                LOG_WARN("Value passed to std::stoull was larger than unsigned long long could represent");
            }
            return res;
        }

    protected:
 
        static bool parse(
                const std::string &content,
                unsigned long long &sessionFirstTimeLaunch,
                std::string &sessionSDKUid)
        {
            if (content.empty()) {
                return false;
            }
            std::vector<std::string> v;
            StringUtils::SplitString(content, '\n', v);
            if (v.size() != 2) {
               return false;
            }
            remove_eol(v[0]);
            remove_eol(v[1]);
            sessionFirstTimeLaunch = convertStrToLong(v[0]);
            if (sessionFirstTimeLaunch == 0 ) {
                return false;
            }
            sessionSDKUid =  v[1];
            return true;
        }

    private:

       static void writeFileContents(
            const std::string &path,
            unsigned long long sessionFirstTimeLaunch,
            const std::string &sessionSDKUid)
        {
            std::string contents;
            contents += std::to_string(sessionFirstTimeLaunch);
            contents += '\n';
            contents += sessionSDKUid;
            contents += '\n';
            if (!MAT::FileWrite(path.c_str(), contents.c_str()))
            {
                LOG_WARN("Unable to save session analytics to %s", path.c_str());
            }
        }

        static void remove_eol(std::string& result)
        {
            if (!result.empty() && result[result.length() - 1] == '\n')
            {
                result.erase(result.length() - 1);
            }
        }
    };
}
ARIASDK_NS_END
#endif



