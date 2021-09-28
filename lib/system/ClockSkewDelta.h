//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#pragma once
#include "pal/PAL.hpp"
#include <vector>
#include <mutex>


namespace MAT_NS_BEGIN {


	class ClockSkewDelta
	{
	public:
		ClockSkewDelta()
		: m_delta("")
		, m_pingSent(false)
		, m_deltaReceived(false)
		{
		}
		
		void SetDelta(const std::string& delta)
		{
			m_deltaReceived = true;
			m_delta = delta;
		}

		bool isClockSkewOn() const
		{
			if (!m_pingSent || (m_deltaReceived && !m_delta.empty()))
			{
				return true;
			}
			return false;
		}

		bool isWaitingForClockSkew() const
		{
			if (!m_deltaReceived && m_pingSent)
			{
				return true;
			}
			return false;
		}

		std::string GetDelta()
		{
			if (m_pingSent == false)
			{
				m_pingSent = true;
				return "use-collector-delta";
			}
			else
				return m_delta;
		}

	RoutePassThrough<ClockSkewDelta, EventsUploadContextPtr const&> encode{ this, &ClockSkewDelta::handleEncode };
	RoutePassThrough<ClockSkewDelta, EventsUploadContextPtr const&> decode{ this, &ClockSkewDelta::handleDecode };

	private:
		std::string			m_delta;
		bool					m_pingSent;
		bool					m_deltaReceived;
		bool handleEncode(EventsUploadContextPtr const& ctx)
		{
			if (!m_delta.empty())
			{
				ctx->httpRequest->GetHeaders().set("time-delta-to-apply-millis", m_delta);
			}
			return true;
		}
		bool handleDecode(EventsUploadContextPtr const& ctx)
		{
			if (isWaitingForClockSkew())
			{
				IHttpResponse const& response = *ctx->httpResponse;
				const HttpHeaders& headers = response.GetHeaders();
				///HttpHeaders::iterator = headers.find("time-delta-millis")

				std::string timeString = headers.get("time-delta-millis");
				if (!timeString.empty())
				{
					SetDelta(timeString);
				}
				else
				{
					SetDelta("");
				}
			}
			return true;
		}
	};

} MAT_NS_END


