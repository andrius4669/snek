#include "config.h"

Config::Config() {
	cfg = cpptoml::parse_file("config.toml");
}

Config::~Config() {}

std::string Config::getLogFilename() const
{
	auto lf = cfg->get_as<std::string>("log.file");
	return lf ? *lf : "log.txt";
}
