//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef EventLogger_H
#define EventLogger_H

#include <map>
#include <string>

class EventLogger
{
public:
	EventLogger(){};
	void Init(std::string token);
	void LogEvent(std::map<std::string, std::string>& event);
	void Pause();
	void Resume();
	void Upload();
	void Done();
};

#endif
