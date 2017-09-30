#include "logger.h"
#include "config.h"
#include <stdarg.h>

Logger::Logger()
{
	auto &cfg = GConf::getInstance();
	auto fn = cfg.getLogFilename();
	if (fn.size())
		logfile = fopen(fn.c_str(),"a");
	else
		logfile = nullptr;
}

Logger::~Logger()
{
	if (logfile)
		fclose(logfile);
}

void Logger::log(const char *s)
{
	fprintf(logfile ? logfile : stderr,"%s",s);
}

void Logger::logf(const char *fmt,...)
{
	va_list args;
	va_start(args,fmt);
	vfprintf(logfile ? logfile : stderr,fmt,args);
}
