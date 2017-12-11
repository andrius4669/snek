#pragma once

#include <string>
#include "cpptoml/include/cpptoml.h"
#include <memory>

struct Config
{
	Config();
	~Config();
	Config(const Config &) = delete;
	Config &operator =(const Config &) = delete;

	static Config &getInstance();

	std::string getLogFilename() const;
private:
	std::shared_ptr<cpptoml::table> cfg;
} ;
typedef Config GConf;
