//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#pragma once
#include "SchemaStub.hpp"

#include <string>

namespace Microsoft {
	namespace Applications 
	{
		// A class that implements this interface can be registered to receive platform events.
		class IPlatformEventReceiver
		{
		public:
			virtual void Suspend() = 0;
			virtual void Resume() = 0;
			virtual void UnhandledException(int code, const std::string& message) = 0;
		};
	}
}

