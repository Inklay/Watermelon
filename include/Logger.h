#pragma once

#define TOML_IMPLEMENTATION
#include "Utils/TOML.h"
#include <fstream>
#include <mutex>

class Logger {
public:
	enum Level {
		DEBUG,
		INFO,
		WARN,
		ERR
	};

	Logger(toml::table& config);
	~Logger();
	void log(const std::string& message, const std::string& caller, Level level);

private:
	toml::table& m_config;
	std::ofstream m_stream;
	std::mutex m_mutex;
	const std::string m_levelTexts[4] = { "DEBUG", "INFO", "WARN", "ERROR" };
};
