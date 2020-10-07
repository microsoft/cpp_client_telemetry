//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef IBACKOFF_HPP
#define IBACKOFF_HPP

#include "pal/PAL.hpp"

#include <memory>
#include <string>

namespace MAT_NS_BEGIN {


/// <summary>
/// Interface for backoff delay calculators.
/// </summary>
class IBackoff {
  public:
    virtual ~IBackoff() {}

    /// <summary>
    /// Resets the backoff back to initial value.
    /// </summary>
    virtual void reset() = 0;

    /// <summary>
    /// Increases the backoff one step higher.
    /// </summary>
    virtual void increase() = 0;

    /// <summary>
    /// Retrieves current backoff value.
    /// Might be randomized a bit each time called if using jitter.
    /// </summary>
    /// <returns>Current backoff value</returns>
    virtual int getValue() = 0;

    /// <summary>
    /// Factory for creating new backoff objects.
    /// </summary>
    /// <returns>Smart pointer to the created backoff object or empty in case
    /// of errors (bad configuration)</returns>
    static std::unique_ptr<IBackoff> createFromConfig(std::string const& config);
};


} MAT_NS_END
#endif

