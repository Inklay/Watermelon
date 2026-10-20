#pragma once

#include <toml++/toml.hpp>
#include <fstream>
#include <mutex>

class Logger {
public:
	enum Level {
		OFF,
		DEBUG,
		INFO,
		WARN,
		ERR
	};

	Logger(toml::table& config);
	~Logger();
	void log(const std::string& message, const std::string& caller, Level level);

private:
	std::ofstream m_stream;
	std::mutex m_mutex;
	const std::string m_levelTexts[5] = { "", "DEBUG", "INFO", "WARN", "ERROR" };
	Level m_minLevel;
	std::string m_timestampFormat;
	bool m_enabled;
	bool m_color;
	bool m_timestamp;
	bool m_thread;
	bool m_threadId;
};
