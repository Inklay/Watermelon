#pragma once

#include <fstream>

class Logger {
public:
	enum Level {
		DEBUG,
		INFO,
		WARN,
		ERR
	};

	Logger(const std::string& path);
	~Logger();
	void log(const std::string& message, const std::string& caller, Level level);

private:
	std::ofstream m_stream;
	const std::string m_levelTexts[4] = { "DEBUG", "INFO", "WARN", "ERROR" };
};
