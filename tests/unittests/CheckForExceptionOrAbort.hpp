//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef CHECKFOREXCEPTIONORABORT_HPP
#define CHECKFOREXCEPTIONORABORT_HPP

#include <ctmacros.hpp>
#include <gtest/gtest.h>

namespace testing {

template<typename ExpectedExceptionType>
void CheckForExceptionOrAbort(std::function<void()>&& functionToCheck) noexcept
{
#if (HAVE_EXCEPTIONS)
	ASSERT_THROW(functionToCheck(), ExpectedExceptionType);
#else
	ASSERT_DEATH(functionToCheck(), "");
#endif // HAVE_EXCEPTIONS
}

} // testing

#endif // CHECKFOREXCEPTIONORABORT_HPP
