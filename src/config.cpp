#include "config.h"

Config::Config() {}

Config::~Config() {}

std::string Config::getLogFilename() const
{
	return "log.txt";
}
