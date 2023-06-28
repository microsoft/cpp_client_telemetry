//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include <random>
#include <ctmacros.hpp>

namespace PAL_NS_BEGIN
{
    // Pseudo-random number generator (not for cryptographic usage).
    // The instances are not thread-safe, serialize access externally if needed.
    class PseudoRandomGenerator {
#if  defined(_WIN32)  || defined (__APPLE__)
    public:
        double getRandomDouble()
        {
            return m_distribution(m_engine);
        }

    protected:
        std::default_random_engine m_engine{ std::random_device()() };
        std::uniform_real_distribution<double> m_distribution{ 0.0, 1.0 };
#else   /* Unfortunately the functionality above fails memory checker on Linux with gcc-5 */
    public:
        double getRandomDouble()
        {
            return ((double)rand() / RAND_MAX);
        }
#endif
    };

} PAL_NS_END
