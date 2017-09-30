#pragma once

#include "singleton.h"
#include <string>

struct Config
{
	Config();
	~Config();

	std::string getLogFilename() const;
} ;
typedef Singleton<Config> GConf;
