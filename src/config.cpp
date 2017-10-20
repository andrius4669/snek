#include "config.h"

Config::Config() {
	try {
		cfg = cpptoml::parse_file("config.toml");
	}
	catch(...) {}
}

Config::~Config() {}

std::string Config::getLogFilename() const
{
	auto defval = "log.txt";
	if (cfg) {
		auto lf = cfg->get_as<std::string>("log.file");
		return lf ? *lf : defval;
	}
	return defval;
}
