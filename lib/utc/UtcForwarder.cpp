// Copyright (c) Microsoft. All rights reserved.

#include "UtcForwarder.hpp"
#include "utils/Common.hpp"
#include "TraceLoggingDynamic.h"
#include "MicrosoftTelemetry.h"

namespace ARIASDK_NS_BEGIN {


static HRESULT createHStringFromUtf8(std::string const& str, Microsoft::WRL::Wrappers::HString& hstr)
{
    int nChars = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0);

    PWSTR strBuffer = NULL;
    HSTRING_BUFFER hStringBuffer = NULL;
    HRESULT hr = WindowsPreallocateStringBuffer(nChars, &strBuffer, &hStringBuffer);
    if (FAILED(hr)) {
        return hr;
    }

    MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), strBuffer, nChars);

    HSTRING hString;
    hr = WindowsPromoteStringBuffer(hStringBuffer, &hString);
    if (FAILED(hr)) {
        WindowsDeleteStringBuffer(hStringBuffer);
        return hr;
    }

    return hstr.Set(hString);
}

static bool tenantIdToGuid(std::string const& tenantId, GUID& guid)
{
    if (tenantId.size() != 32) {
        return false;
    }

    WCHAR buffer[32 + 4 + 1]; // 32 hex chars + 4 dashes + 1 NUL terminator

    size_t i = 0;
    for (char ch : tenantId) {
        if (i == 8 || i == 12 + 1 || i == 16 + 2 || i == 20 + 3) {
            buffer[i++] = L'-';
        }
        buffer[i++] = static_cast<WCHAR>(ch);
    }
    buffer[i] = L'\0';

    return (UuidFromStringW(reinterpret_cast<RPC_WSTR>(buffer), &guid) == 0 /*RPC_S_OK*/);
}

//---

UtcForwarder::UtcForwarder()
{
    using namespace Microsoft::WRL::Wrappers;

    HRESULT hr = Windows::Foundation::Initialize(RO_INIT_MULTITHREADED);
    if (FAILED(hr)) {
        LOG_INFO("WinRT not available (0x%08X), UTC forwarding will be disabled for all tenants",
            hr);
        return;
    }

    hr = Windows::Foundation::GetActivationFactory(HStringReference(RuntimeClass_Windows_System_Diagnostics_Telemetry_PlatformTelemetryClient).Get(), &m_platformTelemetryClient);
    if (FAILED(hr)) {
        LOG_INFO("PlatformTelemetryClient not available (0x%08X), UTC forwarding will be disabled for all tenants",
            hr);
        Windows::Foundation::Uninitialize();
        return;
    }

    LOG_INFO("WinRT and PlatformTelemetryClient are available, UTC forwarding is possible");
}

UtcForwarder::~UtcForwarder()
{
    if (m_platformTelemetryClient) {
        m_platformTelemetryClient.Reset();
        Windows::Foundation::Uninitialize();
    }
}

bool UtcForwarder::registerTenantWithUtc(std::string const& iKey, uint32_t storageSize, uint32_t uploadQuotaSize)
{
    using namespace Microsoft::WRL;
    using namespace Microsoft::WRL::Wrappers;

    RoInitializeWrapper apartment(RO_INIT_MULTITHREADED);

    HString hstrIkey;
    HRESULT hr = createHStringFromUtf8(iKey, hstrIkey);
    if (FAILED(hr)) {
        LOG_ERROR("Could not create iKey HSTRING (0x%08X)",
            hr);
        return false;
    }

    ComPtr<AWSDT::IPlatformTelemetryRegistrationSettings> settings;
    hr = Windows::Foundation::ActivateInstance(HStringReference(RuntimeClass_Windows_System_Diagnostics_Telemetry_PlatformTelemetryRegistrationSettings).Get(), &settings);
    if (FAILED(hr)) {
        LOG_ERROR("Could not create PlatformTelemetryRegistrationSettings instance (0x%08X)",
            hr);
        return false;
    }
    settings->put_StorageSize(storageSize);
    settings->put_UploadQuotaSize(uploadQuotaSize);

    ComPtr<AWSDT::IPlatformTelemetryRegistrationResult> result;
    hr = m_platformTelemetryClient->RegisterWithSettings(hstrIkey.Get(), settings.Get(), &result);
    if (FAILED(hr)) {
        LOG_ERROR("Could not register iKey (0x%08X)",
            hr);
        return false;
    }

    AWSDT::PlatformTelemetryRegistrationStatus status = AWSDT::PlatformTelemetryRegistrationStatus_UnknownFailure;
    hr = result->get_Status(&status);
    if (FAILED(hr) || status != AWSDT::PlatformTelemetryRegistrationStatus_Success) {
        LOG_ERROR("iKey registration result is not Success (0x%08X, %u)",
            hr, status);
        return false;
    }

    return true;
}

bool UtcForwarder::handleForwardIfAvailable(IncomingEventContextPtr const& ctx)
{
    if (!m_platformTelemetryClient) {
        return false;
    }

    std::string iKey     = "P-ARIA-" + ctx->record.tenantToken;
    std::string tenantId = tenantTokenToId(ctx->record.tenantToken);
    GUID tenantIdAsGuid;
    if (!tenantIdToGuid(tenantId, tenantIdAsGuid)) {
        LOG_WARN("Could not parse tenant %s as GUID, the event cannot be forwarded to UTC",
            tenantId.c_str());
        return false;
    }

    bool         registered;
    REGHANDLE    hProvider;
    UINT8 const* providerMetadata;
    {
        LOCKGUARD(m_lock);

        TenantInfo& ti = m_tenantInfo[tenantId];

        if (!ti.seen) {
            ti.seen = true;
            ti.registered = registerTenantWithUtc(iKey, 8 * 1048576, 0);
            if (!ti.registered) {
                LOG_INFO("Could not register tenant %s with PlatformTelemetryClient, UTC forwarding will be disabled for this tenant",
                    tenantId.c_str());
            } else {
                {
                    tld::ProviderMetadataBuilder<std::vector<BYTE>> providerMetaBuilder(ti.providerMetadata);
                    providerMetaBuilder.Begin(("Aria." + tenantId).c_str());
                    static GUID const AriaTelemetryGroup = {0x780dddc8, 0x18a1, 0x5781, {0x89, 0x5a, 0xa6, 0x90, 0x46, 0x4f, 0xa8, 0x9c}};
                    providerMetaBuilder.AddTrait(tld::ProviderTraitGroupGuid, &AriaTelemetryGroup, sizeof(AriaTelemetryGroup));
                    providerMetaBuilder.End();
                }
                HRESULT hr = tld::RegisterProvider(&ti.hProvider, const_cast<GUID*>(&tenantIdAsGuid), ti.providerMetadata.data());
                if (FAILED(hr)) {
                    LOG_WARN("Could not register ETW provider for tenant %s (0x%08X), but PlatformTelemetryClient registration had succeeded, events for this tenant will be dropped",
                        tenantId.c_str(), hr);
                } else {
                    LOG_INFO("Tenant %s was successfully registered with PlatformTelemetryClient and ETW, events for this tenant will be forwarded to UTC",
                        tenantId.c_str());
                }
            }
        }

        registered       = ti.registered;
        hProvider        = ti.hProvider;
        providerMetadata = ti.providerMetadata.data();
    }
    if (hProvider == NULL) {
        if (registered) {
            LOG_ERROR("Event %s/%s should be forwarded to UTC, but provider registration for that tenant had failed, so the event will be dropped",
                tenantId.c_str(), ctx->source->EventType.c_str());
            return true;
        } else {
            return false;
        }
    }


    tld::EventDescriptor eventDescriptor;
    eventDescriptor.Keyword = MICROSOFT_KEYWORD_MEASURES;

    UINT32 eventTags = MICROSOFT_EVENTTAG_NORMAL_PERSISTENCE | MICROSOFT_EVENTTAG_NORMAL_LATENCY;

    // TODO: Use also other keywords and flags based on user's EventProperties.

    std::vector<UINT8> eventMetadata;
    std::vector<UINT8> eventData;
    if (!convertProperties(*ctx->source, iKey, eventTags, eventMetadata, eventData)) {
        LOG_ERROR("Could not convert event %s/%s to UTC format, the even will be dropped",
            tenantId.c_str(), ctx->source->EventType.c_str());
        return true;
    }

    // TODO: Log incoming event stats?

    EVENT_DATA_DESCRIPTOR dataDescriptors[3];
    // The first two descriptors are reserved for use by tld::WriteEvent().
    EventDataDescCreate(&dataDescriptors[2], eventData.data(), static_cast<ULONG>(eventData.size()));
    HRESULT hr = tld::WriteEvent(hProvider, eventDescriptor, providerMetadata, eventMetadata.data(), 3, dataDescriptors);
    if (FAILED(hr)) {
        LOG_ERROR("Event %s/%s could not be forwarded to UTC (0x%08x), the event will be dropped",
            tenantId.c_str(), ctx->source->EventType.c_str(), hr);
        return true;
    }

    LOG_INFO("Event %s/%s forwarded to UTC, keyword 0x%016llX, tags 0x%08X, serialized size %u bytes",
        tenantId.c_str(), ctx->source->EventType.c_str(), eventDescriptor.Keyword, eventTags,
        static_cast<unsigned>(eventMetadata.size() + eventData.size()));

    // TODO: Log sent events stats?

    // Do not continue down the usual route (do not queue for sending to Aria collector).
    return false;
}

bool UtcForwarder::convertProperties(::AriaProtocol::Record& record, std::string const& iKey, UINT32 eventTags, std::vector<UINT8>& metadata, std::vector<UINT8>& data)
{
    tld::EventMetadataBuilder<std::vector<BYTE>> mbuilder(metadata);
    tld::EventDataBuilder<std::vector<BYTE>>     dbuilder(data);
    std::vector<std::string>                     ariaMetadata;

    // TODO: Traverse record.Extension just once, split common properties by prefixes?

    mbuilder.Begin(record.EventType.c_str(), eventTags);

    auto moveExtension = [&record, &dbuilder](char const* srcName, tld::EventMetadataBuilder<std::vector<BYTE>>& mbuilder, char const* dstName, char const* prefix = nullptr) {
            auto it = record.Extension.find(srcName);
            if (it != record.Extension.end()) {
                mbuilder.AddField(dstName, tld::TypeMbcsString);
                dbuilder.AddString(prefix ? (prefix + it->second).c_str() : it->second.c_str());
                record.Extension.erase(it);
            }
        };

    bool const ariaBackCompat = true;

    // Part A extension - backward compatibility
    if (ariaBackCompat) {
        auto partAExtBC = mbuilder.AddStruct("PartAExt_bc");
        partAExtBC.AddField("recType", tld::TypeMbcsString);
        dbuilder.AddString(record.Type.c_str());
        moveExtension("AppInfo.Language",       partAExtBC, "appLang");
        moveExtension("DeviceInfo.Make",        partAExtBC, "make");
        moveExtension("DeviceInfo.Model",       partAExtBC, "model");
        moveExtension("EventInfo.Source",       partAExtBC, "source");
        moveExtension("UserInfo.AdvertisingId", partAExtBC, "advertisingId");
        moveExtension("UserInfo.TimeZone",      partAExtBC, "tz");
    } else {
        record.Extension.erase("AppInfo.Language");
        record.Extension.erase("EventInfo.Source");
        record.Extension.erase("DeviceInfo.Make");
        record.Extension.erase("DeviceInfo.Model");
        record.Extension.erase("UserInfo.AdvertisingId");
        record.Extension.erase("UserInfo.TimeZone");
    }

    // Part A extension - Aria
    auto partAExtAria = mbuilder.AddStruct("PartAExt_aria");
    moveExtension("EventInfo.SdkVersion", partAExtAria, "libVer");
    if (ariaBackCompat) {
        moveExtension("AppInfo.ETag", partAExtAria, "expEtag");
        moveExtension("Session.Id",   partAExtAria, "sesId");
        partAExtAria.AddField("evtTime", tld::TypeInt64);
        dbuilder.AddValue(record.Timestamp);
    }
    record.Extension.erase("AppInfo.ETag");

#if 0
    // TODO: [MG] - let's discuss what we do with these. I'd rather keep these USER DEFINED fields.
    record.Extension.erase("Session.Id");
    record.Extension.erase("Session.State");
    record.Extension.erase("Session.Duration");
    record.Extension.erase("Session.DurationBucket");
    record.Extension.erase("Session.FirstLaunchTime");
    record.Extension.erase("DeviceInfo.SDKUid");
#endif

    // Part A extension - general
    mbuilder.AddField("PartA_iKey", tld::TypeMbcsString);
    dbuilder.AddString(iKey.c_str());
    moveExtension("AppInfo.ExperimentIds",      mbuilder, "PartA_Ext_App_ExpId");
    moveExtension("AppInfo.Id",                 mbuilder, "PartA_Ext_App_Name");
    moveExtension("EventInfo.Sequence",         mbuilder, "PartA_Ext_App_SeqNum");
    // FIXME: e: is for advertising ID, what about skype ID?
    // Also UserInfo.Id is supposed to be in record.PIIExtensions instead.
    moveExtension("UserInfo.Id",                mbuilder, "PartA_Ext_App_UserId", "e:");
    moveExtension("DeviceInfo.NetworkProvider", mbuilder, "PartA_Ext_Net_Provider");
    moveExtension("DeviceInfo.NetworkCost",     mbuilder, "PartA_Ext_Net_Cost");
    moveExtension("DeviceInfo.NetworkType",     mbuilder, "PartA_Ext_Net_Type");
    moveExtension("UserInfo.Language",          mbuilder, "PartA_Ext_Os_Locale");

    record.Extension.erase("AppInfo.Version");
    record.Extension.erase("DeviceInfo.Id");
    record.Extension.erase("DeviceInfo.OsBuild");
    record.Extension.erase("DeviceInfo.OsName");
    record.Extension.erase("DeviceInfo.OsVersion");
    record.Extension.erase("EventInfo.InitId");
    record.Extension.erase("EventInfo.Name");
    record.Extension.erase("EventInfo.Time");
    // FIXME: Remove? This SDK is not setting any of these "S_x" properties.
    record.Extension.erase("S_t");
    record.Extension.erase("S_p");
    record.Extension.erase("S_u");
    record.Extension.erase("S_j");
    record.Extension.erase("S_v");
    record.Extension.erase("S_e");
    record.Extension.erase("UserInfo.ANID");
    record.Extension.erase("UserInfo.MsaId");

    // Extension
    for (auto const& item : record.Extension) {
        std::string name = item.first;
        // FIXME: [MG] - verify that this is correct, cause we'd like the events to flow to ASIMOV in their original namespace
        std::replace(name.begin(), name.end(), '.', '_');
        mbuilder.AddField(name.c_str(), tld::TypeMbcsString);
        dbuilder.AddString(item.second.c_str());
    }

    // PIIExtensions
    for (auto const& item : record.PIIExtensions) {
        std::string name = item.first;
        // FIXME: [MG] - verify that this is correct, cause we'd like the events to flow to ASIMOV in their original namespace
        std::replace(name.begin(), name.end(), '.', '_');
        mbuilder.AddField(name.c_str(), tld::TypeMbcsString);
        dbuilder.AddString(item.second.RawContent.c_str());
        ariaMetadata.push_back("n:" + name + ";s:" + toString(item.second.ScrubType) + ";k:" + toString(item.second.Kind));
    }

    // Part A extension - Aria metadata
    if (!ariaMetadata.empty()) {
        auto partAExtMD = mbuilder.AddStruct("PartA_Ext_AriaMD");
        partAExtMD.AddFieldFixedArray("fields", static_cast<UINT16>(ariaMetadata.size()), tld::TypeMbcsString);
        for (auto const& item : ariaMetadata) {
            dbuilder.AddString(item.c_str());
        }
    }

    if (!mbuilder.End()) {
        // Can fail if the metadata are too large.
        return false;
    }

    return true;
}


} ARIASDK_NS_END
