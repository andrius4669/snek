#pragma once

#include <stdio.h>
#include "singleton.h"

struct Logger
{
	Logger();
	~Logger();

	void log(const char *s);
	void logf(const char *fmt,...);
private:
	FILE *logfile;
} ;
typedef Singleton<Logger> GLog;
