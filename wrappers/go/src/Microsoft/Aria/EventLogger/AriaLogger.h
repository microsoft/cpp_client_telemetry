#ifndef AriaLogger_H
#define AriaLogger_H

#include <map>
#include <string>


class AriaLogger
{
public:
	AriaLogger(){};
	void Init(std::string token);
	void LogEvent(std::map<std::string, std::string>& event);
	void Pause();
	void Resume();
	void Upload();
	void Done();
};

#endif
