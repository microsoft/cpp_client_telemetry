//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef CORRELATIONVECTOR_HPP
#define CORRELATIONVECTOR_HPP

#include "ctmacros.hpp"

#include <mutex>
#include <string>

namespace MAT_NS_BEGIN
{
    // Implementation of the Common Schema standard vector clock type.
    // Class methods are thread-safe.
    // Boolean-value methods return false to indicate failures.

    /*

    Quick start:

        #include <CorrelationVector.hpp>

        // Construct and initialize a correlation vector with a random base value,
        // share that value across the app components which are going to use it.
        // There could be, for example, one CV per app, per scenario, per user.
        CorrelationVector m_appCV;
        m_appCV.Initialize(2);

        // Get the next value, log it and/or pass it to your outgoing service call.
        std::string curCV = m_appCV.GetNextValue();
        EventProperties eventData("Microsoft.OneSDK.Example.HelloWorldEvent");
        eventData.SetProperty(CorrelationVector::PropertyName, curCV);

    Or, if you are receiving CorrelationVector string as an input
    from an upstream caller and would like to keep using and extending it:

        #include <CorrelationVector.hpp>

        // Construct a correlation vector and initialize it with the provided base value.
        CorrelationVector m_appCV;
        m_appCV.SetValue("jj9XLhDw7EuXoC2L");

        // Extend that value.
        m_appCV.Extend();

        // Get the next value, log it and/or pass it to your downstream dependency.
        std::string curCV = m_appCV.GetNextValue();
        EventProperties eventData("Microsoft.OneSDK.Example.HelloWorldEvent");
        eventData.SetProperty(CorrelationVector::PropertyName, curCV);

    */

    class CorrelationVector
    {
    public:

        // Constructs an uninitialized, not yet ready to use correlation vector
        MATSDK_LIBABI CorrelationVector();

        // Initializes CV with a random base value and a current vector count of 0.
        // Version could be 1 or 2, where version allows for the longer base and full
        // CorrelationVector length. Use version 2 only if you are sure that
        // the downstream consumers support it.
        MATSDK_LIBABI bool Initialize(int version);

        // Resets CV value to an uninitialized state.
        // When uninitialized the internal CV value is "" and all Extend/Increment calls are ignored.
        MATSDK_LIBABI void Uninitialize();

        // Returns the flag, specifying whether the CV has been initialized or not.
        MATSDK_LIBABI bool IsInitialized();

        // Atomically reads the current CV string representation and increments it for the next use.
        // Returns the read value or empty string if CV is not initialized.
        MATSDK_LIBABI std::string GetNextValue();

        // Returns the current CV string representation or empty string if not initialized.
        MATSDK_LIBABI std::string GetValue();

        // Adds .0 to the end of the current correlation vector,
        // or does nothing if the maximum length was reached.
        MATSDK_LIBABI bool Extend();

        // Increments the last extension of the correlation vector,
        // or does nothing if the maximum length was reached.
        MATSDK_LIBABI bool Increment();

        // Checks to see if we can add an extra vector.
        // Returns false if the extension will put us over the maximum size of a correlation vector.
        MATSDK_LIBABI bool CanExtend();

        // Checks to see if we can increment the current vector.
        // Returns false if the extension will put us over the maximum size of a correlation vector.
        MATSDK_LIBABI bool CanIncrement();

        // Attempts to set the base and current vector values given the string representation.
        // String could also contain just the base value.
        // Version of CV is auto-detected based on the length of the base value.
        MATSDK_LIBABI bool SetValue(const std::string& cv);

    public:

        // String constant to use for sending a CV through EventProperties.SetProperty API.
        static constexpr const char* PropertyName = "__TlgCV__";

        // String constant to use when sending a CV through any transport other than
        // this telemetry API, e.g. as a name for an HTTP header.
        static constexpr const char* HeaderName = "MS-CV";

    private:

        // Version specific constants.
        static const size_t c_maxCVLength_v1;
        static const size_t c_baseCVLength_v1;

        static const size_t c_maxCVLength_v2;
        static const size_t c_baseCVLength_v2;

        // Helper strings used for input validation.
        static const std::string s_base64CharSet;
        static const std::string s_base10CharSet;
        static const std::string s_maxVectorElementValue;

        // Internal state variables.
        std::mutex m_lock;
        bool m_isInitialized {};
        std::string m_baseVector;
        size_t m_currentVector {};
        size_t m_maxLength {};

        // Randomly generates a string for the base vector.
        static std::string InitializeInternal(size_t baseLength);

        // Internal, unsynchronized class method implementations.
        std::string GetValueInternal();
        bool IncrementInternal();
        size_t GetLengthInternal(size_t vectorValue);
        bool CanExtendInternal();
        bool CanIncrementInternal();

        // Calculates the length of the specified integer.
        size_t GetDigitCount(size_t value);
    };

} MAT_NS_END
#endif

