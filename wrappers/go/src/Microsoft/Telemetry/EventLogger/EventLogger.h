//
// Copyright (c) Microsoft Corporation. All rights reserved.
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
	void UploadMax();
	void Done();
};

#endif
