///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2020 Microsoft Corporation. All rights reserved.
//
// This code is licensed under the MIT License (MIT).
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef IDATAINSPECTOR_HPP
#define IDATAINSPECTOR_HPP

#include "EventProperty.hpp"
#include "IDecorator.hpp"
#include "Version.hpp"
#include "ctmacros.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace MAT_NS_BEGIN
{
    /// <summary>
    /// Enums identifying applicable Data Concerns
    /// Source: https://aka.ms/privacyguard/issuetypes
    /// </summary>
    enum class DataConcernType : uint8_t
    {
        DemographicInfoLanguage = 0,       // The users language ID. Example: En-Us
        DemographicInfoCountryRegion = 1,  // Country/region
        FieldNameImpliesLocation = 2,      // Field name sounds like location data
        InternalEmailAddress = 3,          // SMTP ending with <span>microsoft.com</span>
        PIDKey = 4,                        // Product key
        OutOfScopeIdentifier = 5,          // Client Id for OXO telemetry from the registry
        MachineName = 6,                   // Machine name
        UserDomain = 7,                    // User/Machine domain
        Location = 8,                      // Data appears to specify a location in the real world
        InScopeIdentifier = 9,             // EUPI. Any authenticated identifier of the same types used for DSR.
        InScopeIdentifierActiveUser = 10,  // The current users EUPI for DSR
        IpAddress = 11,                    // Machine’s current IP address
        ExternalEmailAddress = 12,         // SMTP not ending in <span>microsoft.com</span>
        UserName = 13,                     // Current user’s name or part of it.
        UserAlias = 14,                    // Current user’s alias
        Directory = 15,                    // Any directory or file share
        Url = 16,                          // Any URL
        FileNameOrExtension = 17,          // A file extension from the reportable list of extensions (ignores code files)
        Content = 18,                      // Formatted text: HTML, MIME, RTF, Xml, etc.
        FileSharingUrl = 19,               // A URL referencing a common file-sharing site or service.
        Security = 20                      // A URL containing parameters “access_token”, “password”, etc.
    };

    /// <summary>
    /// Common Privacy Contexts to inspect in the data
    /// </summary>
    struct CommonDataContexts final
    {
        /// <summary>
        /// Domain Name for the current machine
        /// </summary>
        std::string DomainName;

        /// <summary>
        /// Friendly Machine Name
        /// </summary>
        std::string MachineName;

        /// <summary>
        /// Unique UserName such as the log-in name
        /// </summary>
        std::string UserName;

        /// <summary>
        /// Unique User Alias, if different than UserName
        /// </summary>
        std::string UserAlias;

        /// <summary>
        /// IP Addresses for local network ports such as IPv4, IPv6, etc.
        /// </summary>
        std::vector<std::string> IpAddresses;

        /// <summary>
        /// Collection of Language identifiers
        /// </summary>
        std::vector<std::string> LanguageIdentifiers;

        /// <summary>
        /// Collection of Machine ID such as Machine Name, Motherboard ID, MAC Address, etc.
        /// </summary>
        std::vector<std::string> MachineIds;

        /// <summary>
        /// Collection of OutOfScope Identifiers such as SQM_ID, Client_ID, etc.
        /// </summary>
        std::vector<std::string> OutOfScopeIdentifiers;
    };

    /// <summary>
    /// This interface allows SDK users to register a data inspector
    /// that will inspect the data being uploaded by the SDK.
    /// </summary>
    class IDataInspector : public IDecorator, public IModule
    {
       public:
        /// <summary>
        /// Default virtual destructor
        /// </summary>
        virtual ~IDataInspector() = default;

        /// <summary>
        /// Set the enabled state at runtime for the inspector.
        /// </summary>
        /// <param name="isEnabled">Boolean value to denote whether the inspector is enabled or not.</param>
        virtual void SetState(bool isEnabled) noexcept = 0;

        /// <summary>
        /// Get the current state for the inspector.
        /// </summary>
        /// <returns>True if the data inspector is enabled, False otherwise.</returns>
        virtual bool GetState() const noexcept = 0;

        /// <summary>
        /// Iterate and inspect the given record's Part-B and
        /// Part-C properties
        /// </summary>
        /// <param name="record">Record to inspect</param>
        /// <returns>Always returns true.</returns>
        virtual bool decorate(::CsProtocol::Record& record) noexcept override = 0;

        /// <summary>
        /// Set Common Privacy Context after initialization.
        /// <b>Note:</b> Data that may have been sent before this method was called
        /// will not be inspected.
        /// </summary>
        /// <param name="freshCommonPrivacyContext">Unique Ptr for Common Privacy Contexts. If the param is nullptr, this method is no-op.</param>
        virtual void AppendCommonDataContext(std::unique_ptr<CommonDataContexts>&& freshCommonPrivacyContext) = 0;

        /// <summary>
        /// Inspect an ISemanticContext value.
        /// </summary>
        /// <param name="semanticContext">Semantic Context to inspect</param>
        virtual void InspectSemanticContext(const std::string& contextName, const std::string& contextValue, bool isGlobalContext, const std::string& associatedTenant) noexcept = 0;

        /// <summary>
        /// Inspect an ISemanticContext value.
        /// </summary>
        /// <param name="semanticContext">Semantic Context to inspect</param>
        virtual void InspectSemanticContext(const std::string& contextName, GUID_t contextValue, bool isGlobalContext, const std::string& associatedTenant) noexcept = 0;

        /// <summary>
        /// Custom inspector to validate wstrings for a given tenant.
        /// </summary>
        /// <param name="customInspector">Function to inspect the given string</param>
        virtual void AddCustomStringValueInspector(std::function<DataConcernType(const std::string& valueToInspect, const std::string& tenantToken)>&& customInspector) noexcept = 0;

        /// <summary>
        /// Custom inspector to validate GUIDs for a given tenant.
        /// </summary>
        /// <param name="customInspector">Function to inspect the given GUID</param>
        virtual void AddCustomGuidValueInspector(std::function<DataConcernType(GUID_t valueToInspect, const std::string& tenantToken)>&& customInspector) noexcept = 0;

        /// <summary>
        /// Add known concerns for a given event and field pairs.
        /// If a concern is identified in this field for the same Privacy Issue Type,
        /// it is ignored
        /// </summary>
        /// <param name="ignoredConcernsCollection">Collection of ignored concerns</param>
        /// <returns></returns>
        virtual void AddIgnoredConcern(const std::vector<std::tuple<std::string /*EventName*/, std::string /*FieldName*/, DataConcernType /*IgnoredConcern*/>>& ignoredConcernsCollection) noexcept = 0;
    };

}
MAT_NS_END

#endif
