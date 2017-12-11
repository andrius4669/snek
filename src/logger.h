#pragma once

#include <stdio.h>

struct Logger
{
	Logger();
	~Logger();
	Logger(const Logger &) = delete;
	Logger &operator =(const Logger &) = delete;

	static Logger &getInstance();

	void log(const char *s);
	void logf(const char *fmt,...);
private:
	FILE *logfile;
} ;

typedef Logger GLog;
