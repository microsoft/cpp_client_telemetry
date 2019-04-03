#pragma once
class NativeLogger
{
public:
	NativeLogger();
	virtual ~NativeLogger();

	void Start(bool useUtc);
	void Stop();
	void LogEvents(int count, bool isCritical, bool isRealtime);

protected:

	void * CreateLogger(bool useUtc);

protected:
	void * m_pLogger;
	int m_eventCount;
};

