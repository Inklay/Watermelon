#include <sstream>
#include <chrono>
#include <iostream>
#include <format>
#include <thread>
#include "Core/Logger.h"

Logger::Logger(toml::table& config) :
	m_stream(config["file"].value_or("latest.log"), std::ios::out | std::ios::trunc),
	m_timestampFormat(config["timestamp_format"].value_or<std::string>("[hour]:[minute]:[second]")),
	m_enabled(config["enabled"].value_or(true)),
	m_color(config["color"].value_or(true)),
	m_timestamp(config["timestamp"].value_or(true)),
	m_thread(config["threads"].value_or(true)),
	m_threadId(config["thread_ids"].value_or(false))
{
	if (config["level"].value_or<std::string>("info") == "off") {
		m_minLevel = OFF;
	} else if(config["level"].value_or<std::string>("info") == "debug") {
		m_minLevel = DEBUG;
	} else if (config["level"].value_or<std::string>("info") == "warn") {
		m_minLevel = WARN;
	} else if (config["level"].value_or<std::string>("info") == "error") {
		m_minLevel = ERR;
	} else {
		m_minLevel = INFO;
	}

	if (m_stream.bad()) {
		log("Can't create log file '" + config["file"].value_or<std::string>("latest.log") + "'", "Watermelon", ERR);
	}
}

void Logger::log(const std::string& message, const std::string& caller, Level level) {
	if (!m_enabled || m_minLevel == OFF) {
		return;
	}

	if (level > ERR || level < DEBUG) {
		return;
	}

	if (level < m_minLevel) {
		return;
	}

	std::stringstream log;
	std::stringstream fileLog;
	auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
	auto local = std::chrono::current_zone()->to_local(now);

	if (m_timestamp) {
		std::string format = m_timestampFormat;
		size_t index = format.find("[hour]");

		if (index != std::string::npos) {
			format = format.replace(index, 6, "%H");
		}

		index = format.find("[minute]");

		if (index != std::string::npos) {
			format = format.replace(index, 8, "%M");
		}

		index = format.find("[second]");

		if (index != std::string::npos) {
			format = format.replace(index, 8, "%S");
		}

		if (m_color) {
			log << "\033[32m";
		}

		std::string formated;
		try {
			formated = std::vformat("[{:" + format + "}]", std::make_format_args(local));
		} catch (...) {
			formated = std::format("[{:%H:%M:%S}]", local);
		}

		log << formated;
		fileLog << formated;
	}

	if (m_color) {
		switch (level) {
		case DEBUG:
			log << "\033[36m";
			break;
		case INFO:
			log << "\033[35m";
			break;
		case WARN:
			log << "\033[33m";
			break;
		case ERR:
			log << "\033[31m";
			break;
		default:
			log << "\033[30m";
			break;
		}
	}

	log << " [";
	fileLog << " [";

	if (m_thread) {
		log << caller << "/";
		fileLog << caller << "/";
	}

	if (m_threadId) {
		log << std::this_thread::get_id() << "/";
		fileLog << std::this_thread::get_id() << "/";
	}

	log << m_levelTexts[level] << "]\033[0m: " << message << std::endl;
	fileLog << m_levelTexts[level] << "]: " << message << std::endl;

	std::lock_guard<std::mutex> lock(m_mutex);
	std::cout << log.str();

	if (m_stream.is_open()) {
		m_stream << fileLog.str();
		m_stream.flush();
	}
}
