//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef BACKOFF_JITTER_HPP
#define BACKOFF_JITTER_HPP

#include "backoff/IBackoff.hpp"
#include "pal/PseudoRandomGenerator.hpp"

#include <algorithm>
#include <assert.h>
#include <math.h>

namespace MAT_NS_BEGIN {


/// <summary>
/// Backoff implementation with exponential steps and added jitter.
/// </summary>
class Backoff_ExponentialWithJitter : public IBackoff {
  public:
    /// <summary>
    /// Initializes the exponential backoff calculator.
    /// Generated value for step <c>N</c> will be in range <c>[initialValue *
    /// multiplier^N, initialValue * multiplier^(N + jitter)]</c>, except
    /// near <c>maximumValue</c>, where jitter range is given priority over
    /// the range starting value. The output will be always clipped to
    /// <c>[initialValue, maximumValue].</c>
    /// </summary>
    /// <param name="initialValue">Minimum (first) backoff value</param>
    /// <param name="maximumValue">Maximum possible (last) backoff value</param>
    /// <param name="multiplier">Multiplier for each backoff increase</param>
    /// <param name="jitter">Optional jitter factor, 0.0 means no jitter</param>
    Backoff_ExponentialWithJitter(int initialValue, int maximumValue, double multiplier, double jitter)
      : m_initialValue(initialValue),
        m_maximumValue(maximumValue),
        m_multiplier(multiplier),
        m_jitter(jitter)
    {
        increase_private();
    }

    bool good() const
    {
        return (m_initialValue >= 0) && (m_initialValue <= m_maximumValue) && (m_multiplier > 1.0) && (m_jitter >= 0.0);
    }

    virtual void reset() override
    {
        reset_private();
    }

    virtual void increase() override
    {
        increase_private();
    }

    virtual int getValue() override
    {
        double value = m_currentBase;

        if (m_currentRange > 0) {
            value += m_rand.getRandomDouble() * m_currentRange;
        }

        return static_cast<int>(floor(value));
    }

  protected:
    double m_initialValue {}, m_maximumValue {}, m_multiplier {}, m_jitter {};
    double m_currentBase {}, m_currentRange {}, m_step {};
    PAL::PseudoRandomGenerator m_rand;

private:
    // Private implementation of reset--exists to avoid calling virtual methods in ctor
    void reset_private()
    {
        m_currentBase = 0.0;
        m_currentRange = 0.0;
        m_step = 0.0;
        increase_private();
    }

    // Private implementation of increase--exists to avoid calling virtual methods in ctor
    void increase_private()
    {
        if (m_currentBase + m_currentRange >= m_maximumValue) {
            return;
        }

        m_currentBase = floor(m_initialValue * pow(m_multiplier, m_step));
        m_currentRange = (m_jitter > 0.0) ? floor(m_initialValue * pow(m_multiplier, m_step + m_jitter) - m_currentBase) : 0.0;

        if (m_currentBase + m_currentRange > m_maximumValue) {
            m_currentBase = std::max(m_initialValue, m_maximumValue - m_currentRange);
            m_currentRange = std::min(m_currentRange, m_maximumValue - m_currentBase);
        }

        m_step += 1.0;
    }
};


} MAT_NS_END
#endif

