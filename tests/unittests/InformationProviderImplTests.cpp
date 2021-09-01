//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"
#include "InformationProviderImpl.hpp"

#include <utility>
#include <vector>

using namespace testing;
using namespace MAT;
using namespace PAL;

namespace
{
    class PropertyChangedCallbackSink : public IPropertyChangedCallback
    {
        std::vector<std::pair<std::string, std::string>> callbacks;

        void OnChanged(std::string const& propertyName, std::string const& propertyValue) override
        {
            callbacks.emplace_back(propertyName, propertyValue);
        }

    public:
        PropertyChangedCallbackSink() = default;

        const auto& GetCallbacks() { return callbacks; }
    };
}

TEST(InformationProviderImplTests, ConstructionTest)
{
    InformatonProviderImpl informationProvider;
}

TEST(InformationProviderImplTests, RegisterTest)
{
    PropertyChangedCallbackSink callbackSink;
    InformatonProviderImpl informationProvider;
    int registrationCookie = informationProvider.RegisterInformationChangedCallback(&callbackSink);

    informationProvider.OnChanged("PropertyName", "PropertyValue1");

    // Assert that a single callback has been made, and the correct values were received
    ASSERT_EQ(1u, callbackSink.GetCallbacks().size());
    ASSERT_EQ((std::pair<std::string, std::string>{"PropertyName", "PropertyValue1"}), callbackSink.GetCallbacks()[0]);

    informationProvider.UnRegisterInformationChangedCallback(registrationCookie);
}

TEST(InformationProviderImplTests, RegisterUnregisterTest)
{
    PropertyChangedCallbackSink callbackSink;
    InformatonProviderImpl informationProvider;
    int registrationCookie = informationProvider.RegisterInformationChangedCallback(&callbackSink);

    informationProvider.OnChanged("PropertyName", "PropertyValue1");

    ASSERT_EQ(1u, callbackSink.GetCallbacks().size());

    informationProvider.UnRegisterInformationChangedCallback(registrationCookie);
    informationProvider.OnChanged("PropertyName", "PropertyValue2");

    // After the registration has been removed only the original callback should've been received, the second notification should *not* be received.
    ASSERT_EQ(1u, callbackSink.GetCallbacks().size());
}
