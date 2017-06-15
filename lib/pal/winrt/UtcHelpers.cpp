#pragma once
#include "pal/UtcHelpers.hpp"
#include <string>
#include <PlatformHelpers.h>
using namespace ::Windows::System::Diagnostics::Telemetry;
using namespace std;

namespace ARIASDK_NS_BEGIN {
	namespace PAL {

		bool IsUtcRegistrationEnabledinWindows()
		{
			return ::Windows::Foundation::Metadata::ApiInformation::IsTypePresent("Windows.System.Diagnostics.Telemetry.PlatformTelemetryClient");
		}

		bool RegisterIkeyWithWindowsTelemetry(std::string const& ikey, int storageSize, int uploadQuotaSize)
		{
			PlatformTelemetryRegistrationSettings^ settings = ref new PlatformTelemetryRegistrationSettings();
			settings->StorageSize = storageSize;
			settings->UploadQuotaSize = uploadQuotaSize;

			int size = MultiByteToWideChar(CP_UTF8, 0, ikey.c_str(), (int)ikey.size(), NULL, 0);
			std::wstring wstrTo(size, 0);
			MultiByteToWideChar(CP_UTF8, 0, ikey.c_str(), (int)ikey.size(), &wstrTo[0], size);

			Platform::String^ ikeyP = ref new Platform::String(wstrTo.c_str());

			PlatformTelemetryRegistrationResult^ result = ::Windows::System::Diagnostics::Telemetry::PlatformTelemetryClient::Register(ikeyP, settings);

			delete settings;
			delete ikeyP;

			if (result->Status == PlatformTelemetryRegistrationStatus::Success)
			{
				delete result;
				return true;
			}
			else
			{
				delete result;
				return false;
			}
		}

	}// namespace PAL
} ARIASDK_NS_END