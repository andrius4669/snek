#include "config.h"
#include <stdio.h>

Config::Config() {
	try {
		cfg = cpptoml::parse_file("config.toml");
		fprintf(stderr,"config.toml loading succeeded\n");
	}
	catch (...) {
		fprintf(stderr,"config.toml loading failed\n");
	}
}

Config::~Config() {}

Config &Config::getInstance()
{
	static Config instance;
	return instance;
}

std::string Config::getLogFilename() const
{
	if (cfg) {
		auto lf = cfg->get_as<std::string>("log.file");
		return lf ? *lf : "";
	}
	return "";
}
