#if 0
#define LOG_MODULE DBG_TPM
#include "common/TraceHelper.hpp"
#include "ClockSkewManager.hpp"
#include "ctsdk/palhelper.hpp"

namespace common
{

    ClockSkewManager::ClockSkewManager()
        : m_delta("")
        , m_pingSent(false)
        , m_deltaReceived(false)
        , m_resumeTransmissionAfterClockSkew(false)
    {
        TRACE("ctor ClockSkewManager")
    }

    ClockSkewManager::~ClockSkewManager()
    {
        TRACE("~dtor ClockSkewManager")
    }

    void ClockSkewManager::SetDelta(std::string delta)
    {
        {
            LOCKGUARD(m_lock);
            m_deltaReceived = true;
            m_delta = delta;
        }
    }

    bool ClockSkewManager::isClockSkewOn()
    {
        {
            LOCKGUARD(m_lock);
            if (!m_pingSent || (m_deltaReceived && !m_delta.empty()))
            {
                return true;
            }
            return false;
        }
    }

    bool ClockSkewManager::isWaitingForClockSkew()
    {
        {
            LOCKGUARD(m_lock);
            if (!m_deltaReceived && m_pingSent)
            {
                return true;
            }
            return false;
        }
    }

    std::string ClockSkewManager::GetDelta()
    {
        {
            LOCKGUARD(m_lock);
            if (m_pingSent == false)
            {
                m_pingSent = true;
                return "use-collector-delta";
            }
            else
                return m_delta;
        }
    }

	bool ClockSkewManager::GetResumeTransmissionAfterClockSkew()
	{
		return m_resumeTransmissionAfterClockSkew;
	}

	void ClockSkewManager::SetResumeTransmissionAfterClockSkew(bool value)
	{
		m_resumeTransmissionAfterClockSkew = value;
	}
}
#endif
