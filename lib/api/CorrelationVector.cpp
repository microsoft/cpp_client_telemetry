//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "CorrelationVector.hpp"
#include "utils/StringUtils.hpp" // for SplitString and AreAllCharactersWhitelisted

#include <vector>
#include <random>
#include <limits>

using std::string;
using std::mutex;
using std::vector;

namespace MAT_NS_BEGIN
{
    // Note: CV spec reserves the last character for the "!" suffix identifying sealed values.
    // This effectively means we have one less character to use.
    // (so 64 is reduced to 63 for v1 and 128 is reduced to 127 for v2).

    const size_t CorrelationVector::c_maxCVLength_v1 = 63;
    const size_t CorrelationVector::c_baseCVLength_v1 = 16;

    const size_t CorrelationVector::c_maxCVLength_v2 = 127;
    const size_t CorrelationVector::c_baseCVLength_v2 = 22;

    const string CorrelationVector::s_base64CharSet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const string CorrelationVector::s_base10CharSet = "0123456789";
    const string CorrelationVector::s_maxVectorElementValue = "4294967295";
    
    CorrelationVector::CorrelationVector()
    {
    }
    
    bool CorrelationVector::Initialize(int version)
    {
        std::lock_guard<mutex> lock(m_lock);
        
        if (version != 1 && version != 2)
        {
            return false;
        }
        
        m_maxLength = (version == 1 ? c_maxCVLength_v1 : c_maxCVLength_v2);
        size_t baseLength = (version == 1 ? c_baseCVLength_v1 : c_baseCVLength_v2);
        
        m_currentVector = 0;
        m_baseVector = InitializeInternal(baseLength);
        m_isInitialized = true;
        
        return true;
    }
    
    void CorrelationVector::Uninitialize()
    {
        std::lock_guard<mutex> lock(m_lock);
        
        m_currentVector = 0;
        m_baseVector = "";
        m_isInitialized = false;
    }
    
    bool CorrelationVector::IsInitialized()
    {
        return m_isInitialized;
    }
    
    string CorrelationVector::InitializeInternal(size_t baseLength)
    {
        string result = "";
        std::random_device randomDevice;
        std::uniform_int_distribution<int> base64Dist(0, 63);
        
        
        for (size_t i = 0; i < baseLength; i++)
        {
            result += s_base64CharSet[base64Dist(randomDevice)];
        }
        
        return result;
    }

    string CorrelationVector::GetNextValue()
    {
        std::lock_guard<mutex> lock(m_lock);

        string result = GetValueInternal();
        IncrementInternal();

        return result;
    }

    string CorrelationVector::GetValue()
    {
        std::lock_guard<mutex> lock(m_lock);
        
        return GetValueInternal();
    }
    
    string CorrelationVector::GetValueInternal()
    {
        if (m_isInitialized)
        {
            return  m_baseVector + "." + std::to_string(m_currentVector);
        }
        return string { };
    }

    bool CorrelationVector::Extend()
    {
        std::lock_guard<mutex> lock(m_lock);
        
        bool result = false;

        if (CanExtendInternal())
        {
            m_baseVector = GetValueInternal();
            m_currentVector = 0;
            result = true;
        }
        
        return result;
    }

    bool CorrelationVector::Increment()
    {
        std::lock_guard<mutex> lock(m_lock);
        
        return IncrementInternal();
    }

    bool CorrelationVector::IncrementInternal()
    {
        bool result = false;

        if (CanIncrementInternal())
        {
            m_currentVector++;
            result = true;
        }

        return result;
    }
    
    bool CorrelationVector::CanExtend()
    {
        std::lock_guard<mutex> lock(m_lock);
        
        return CanExtendInternal();
    }
    
    bool CorrelationVector::CanIncrement()
    {
        std::lock_guard<mutex> lock(m_lock);
        
        return CanIncrementInternal();
    }

    bool CorrelationVector::CanExtendInternal()
    {
        if (!m_isInitialized)
        {
            return false;
        }
        
        // extending is appending ".0"
        size_t newLength = GetLengthInternal(m_currentVector) + 2;

        return (newLength <= m_maxLength);
    }

    bool CorrelationVector::CanIncrementInternal()
    {
        if (!m_isInitialized)
        {
            return false;
        }
        
        if (m_currentVector == std::numeric_limits<unsigned int>::max())
        {
            return false;
        }

        // incrementing is adding one to m_currentVector
        size_t newLength = GetLengthInternal(m_currentVector + 1);
        
        return (newLength <= m_maxLength);
    }
    
    size_t CorrelationVector::GetLengthInternal(size_t vectorValue)
    {
        size_t vectorSize = GetDigitCount(vectorValue);
        
        // base + "." + numeric representation of m_currentVector
        return (m_baseVector.length() + 1 + vectorSize);
    }
    
    size_t CorrelationVector::GetDigitCount(size_t value)
    {
        size_t digitCount = 1;
        
        while (value > 9)
        {
            value /= 10;
            digitCount++;
        }
        
        return digitCount;
    }
    
    bool CorrelationVector::SetValue(const string& cv)
    {
        std::lock_guard<mutex> lock(m_lock);
        
        // handle a special case: the last character could a "!", meaning that the vector is sealed for extension
        if ((cv.length() == c_maxCVLength_v1 + 1 || cv.length() == c_maxCVLength_v2 + 1) && cv[cv.length() - 1] == '!')
        {

        }

        vector<string> parts;
        StringUtils::SplitString(cv, '.', parts);
        
        if (parts.size() == 0)
        {
            return false;
        }
        
        size_t maxLength = 0;
        for (size_t i = 0; i < parts.size(); i++)
        {
            // the first character group must be a base64 string or a certain length
            if (i == 0)
            {
                if (parts[i].length() == c_baseCVLength_v1)
                {
                    maxLength = c_maxCVLength_v1;
                }
                else if (parts[i].length() == c_baseCVLength_v2)
                {
                    maxLength = c_maxCVLength_v2;
                }
                else
                {
                    return false;
                }
                    
                if (!StringUtils::AreAllCharactersWhitelisted(parts[i], s_base64CharSet))
                {
                    return false;
                }
            }
            
            // all other character groups must be non-empty, decimal digits
            if (i != 0 && (parts[i].length() == 0 || !StringUtils::AreAllCharactersWhitelisted(parts[i], s_base10CharSet)))
            {
                return false;
            }
        }
        
        // validate the full cV length based on the known cV version
        if (cv.length() > maxLength)
        {
            return false;
        }

        if (parts.size() == 1)
        {
            // init with just the base value
            m_baseVector = parts[0];
            m_currentVector = 0;
        }
        else
        {
            size_t lastDot = cv.find_last_of(".");
            bool parsingFailed = false;
            string vectorString = cv.substr(lastDot + 1, string::npos);
            // note: unsigned long is 32-bit on 32-bit arm devices
            unsigned long currentVector = 0;
            
            try
            {
                // do a manual string comparison before trying to parse the value to avoid throwing an exception
                if (vectorString.length() == 0 ||
                    vectorString.length() > s_maxVectorElementValue.length() ||
                    (vectorString.length() == s_maxVectorElementValue.length() && vectorString > s_maxVectorElementValue))
                {
                    parsingFailed = true;
                }
                else
                {
                    currentVector = std::stoul(vectorString);
                }
            }
            catch (std::invalid_argument&)
            {
                parsingFailed = true;
            }
            catch (std::out_of_range&)
            {
                parsingFailed = true;
            }
            
            if (parsingFailed || currentVector > std::numeric_limits<unsigned int>::max())
            {
                return false;
            }
            
            m_baseVector = cv.substr(0, lastDot);
            m_currentVector = currentVector;
        }
        
        m_maxLength = maxLength;
        m_isInitialized = true;
        
        return true;
    }

} MAT_NS_END

