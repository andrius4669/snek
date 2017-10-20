#pragma once

#include "singleton.h"
#include <string>
#include "cpptoml/include/cpptoml.h"
#include <memory>

struct Config
{
	Config();
	~Config();

	std::string getLogFilename() const;
private:
	std::shared_ptr<cpptoml::table> cfg;
} ;
typedef Singleton<Config> GConf;
