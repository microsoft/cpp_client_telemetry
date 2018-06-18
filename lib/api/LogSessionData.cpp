// Copyright (c) Microsoft. All rights reserved.

#include <string.h>

#include "LogSessionData.hpp"
#include "pal/PAL.hpp"
#include "Logger.hpp"
#include "DebugEvents.hpp"
#include "offline/FifoFileStorage.hpp"
#include <memory>

// TODO: [MG] - verify this functionality, esp. that the file is created where expected
namespace ARIASDK_NS_BEGIN {
        
    LogSessionData::LogSessionData(std::string const& cacheFilePath)
    {
        std::string sessionPath = cacheFilePath.empty() ? "" : (cacheFilePath + ".session").c_str();
        if (!sessionPath.empty())
        {
            int* xPtr = nullptr;
            int IntptrSize = sizeof(xPtr);
            if (IntptrSize > 4) // on 64 bit system, we want session to have different file because FIFO has trouble opening 32 bit file in 64 bit mode
            {
                sessionPath = sessionPath + "64";
            }
            UNREFERENCED_PARAMETER(xPtr);
            //LOG_TRACE("%s: sessionPath=%s", __FUNCTION__, sessionPath.c_str());
            if (StartSessionStorage(sessionPath)) {
                PopulateSession();
                StopSessionStorage();
            }
        }
    }

    unsigned long long LogSessionData::getSesionFirstTime() const
    {
        //LOG_DEBUG("ENTER/EXIT %s", __FUNCTION__);
        return m_sessionFirstTimeLaunch;
    }

    std::string& LogSessionData::getSessionSDKUid()
    {
        //LOG_DEBUG("ENTER/EXIT %s", __FUNCTION__);
        return m_sessionSDKUid;
    }

  
    bool LogSessionData::StartSessionStorage(std::string const& sessionpath)
    {
        m_sessionStorage = new FIFOFileStorage();
        if (m_sessionStorage == NULL)
        {
            LOG_ERROR("Start: Failed to create FifiFileStorage");
            StopSessionStorage();
            return false;
        }
        LOG_TRACE("%s: FifiFileStorage initialized with path %s", __FUNCTION__, sessionpath.c_str());

        // 2 blocks by 32768 should be enough for session storage
        if (m_sessionStorage->Open(sessionpath.c_str(), 32768 * 2) != 0) {
            LOG_ERROR("Start: Failed to open FifoStorage");
            StopSessionStorage();
            return false;
        }

        LOG_TRACE("%s: FifiFileStorage started.", __FUNCTION__);
        return true;
    }

    void LogSessionData::StopSessionStorage()
    {
        if (m_sessionStorage != NULL)
        {
            m_sessionStorage->Flush();
            m_sessionStorage->Close();
            delete m_sessionStorage;
            m_sessionStorage = NULL;
        }
    }

        /******************************************************************************
        * LogSessionData::_PopulateSession
        *
        * Populate Session SDKUid or create it if it doesn't exist
        * Populate FirstTimeLauch or create it if it doesn't exist
        *
        * TODO: replace with a nicer Session Storage key-value store API
        *
        ******************************************************************************/

    unsigned long long LogSessionData::to_long(const char *string, size_t size)
    {
        unsigned long long number = 0;

        for (size_t index = 0; index < size; index++)
            if (string[index] < '0' || string[index] > '9')
                return 0;
            else
                number = number * 10 + string[index] - '0';

        return number;
    }

    void LogSessionData::PopulateSession()
    {

        bool sdkuidFound = false;
        bool timeFound = false;

        static char buf[1024] = { 0 };
        static char *buffer;
        size_t resultSize;

        DATARV_ERROR errorCode = DATARV_ERROR::DATARV_ERROR_OK;
        m_sessionFirstTimeLaunch = 0;
        FindItemInfo findItemInfo = {};
        errorCode = m_sessionStorage->FindFirstItem(&findItemInfo);
        while (!errorCode)
        {
            LOG_TRACE("%s: key=%s", __FUNCTION__, (char *)(findItemInfo.Key.CustomProperty3));

            buffer = (char *)(&buf[0]);

            // Pass valid buffer ptr, but the ReadItem may change it and return a different buffer
            errorCode = m_sessionStorage->ReadItem(findItemInfo, sizeof(buf), &buffer, &resultSize);

            if (resultSize == 0)
            {
                LOG_ERROR("Corrupt value!");
                errorCode = DATARV_ERROR::DATARV_ERROR_INIT_OFFLINESTORAGE_FAILED;
                continue;
            }

            // Save a copy back if needed
            memcpy(buf, buffer, resultSize);
            // ...and NUL-terminate it because it is not NUL-terminated in DB
            buf[resultSize] = 0;
            buffer = (char *)(&buf[0]);
            LOG_TRACE("%s: value=%s", __FUNCTION__, buffer);

            if (strcmp(findItemInfo.Key.CustomProperty3, SESSION_FIRST_TIME) == 0)
            {
                m_sessionFirstTimeLaunch = to_long(buffer, resultSize);
                timeFound = (m_sessionFirstTimeLaunch != 0);
            }
            else if (strcmp(findItemInfo.Key.CustomProperty3, SESSION_SDKU_ID) == 0)
            {
                m_sessionSDKUid = std::string(buffer);
                sdkuidFound = true;
            }
            errorCode = m_sessionStorage->FindNextItem(&findItemInfo);
        }

        if (!timeFound) {
            // Populate the first time launch
            StorageItemKey fileItemInfo = {};
            strncpy_s(fileItemInfo.CustomProperty3, sizeof(fileItemInfo.CustomProperty3), SESSION_FIRST_TIME, strlen(SESSION_FIRST_TIME));
            LOG_TRACE("%s: CustomProperty3=%s", __FUNCTION__, (char *)(&fileItemInfo.CustomProperty3[0]));
            m_sessionFirstTimeLaunch = PAL::getUtcSystemTimeMs();
            std::string timeNow = std::to_string(m_sessionFirstTimeLaunch);
            if (m_sessionStorage->SaveItem(timeNow.c_str(), timeNow.length(), &fileItemInfo))
            {
                LOG_ERROR("Unable to save SESSION_FIRST_TIME");
                m_sessionFirstTimeLaunch = 0;
                return;
            }
            LOG_TRACE("%s: item saved: timeNow=%s", __FUNCTION__, timeNow.c_str());
        }

        if (!sdkuidFound)
        {
            // Populate SDK UUID
            StorageItemKey fileItemInfo = {};
            strncpy_s(fileItemInfo.CustomProperty3, sizeof(fileItemInfo.CustomProperty3), SESSION_SDKU_ID, strlen(SESSION_SDKU_ID));
            LOG_TRACE("%s: CustomProperty3=%s", __FUNCTION__, (char *)(&fileItemInfo.CustomProperty3[0]));
            m_sessionSDKUid = PAL::generateUuidString();
            if (m_sessionStorage->SaveItem(m_sessionSDKUid.c_str(), m_sessionSDKUid.length(), &fileItemInfo))
            {
                LOG_ERROR("Unable to save SDK UUID");
                m_sessionSDKUid = "";
                return;
            }
            LOG_TRACE("%s: item saved: m_sessionSDKUid=%s", __FUNCTION__, m_sessionSDKUid.c_str());
        }
    }
} ARIASDK_NS_END
