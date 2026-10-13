#include <sstream>
#include <chrono>
#include <iostream>
#include <format>
#include "Logger.h"

Logger::Logger(toml::table& config) :
	m_config(config),
	m_stream(config["file"].value_or("latest.log"), std::ios::out | std::ios::trunc)
{
	if (m_stream.bad()) {
		log("Can't create log file '" + m_config["file"].value_or<std::string>("latest.log") + "'", "Watermelon", ERR);
	}
}

Logger::~Logger() {
	m_stream.close();
}

void Logger::log(const std::string& message, const std::string& caller, Level level) {
	if (!m_config["enabled"].value_or(true) || m_config["level"].value_or("info") == "off") {
		return;
	}

	if (level > ERR || level < DEBUG) {
		return;
	}

	if (level < INFO && m_config["level"].value_or<std::string>("info") == "info") {
		return;
	}

	if (level < WARN && m_config["level"].value_or<std::string>("info") == "warn") {
		return;
	}

	if (level < ERR && m_config["level"].value_or<std::string>("info") == "error") {
		return;
	}

	std::stringstream log;
	auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
	auto local = std::chrono::current_zone()->to_local(now);

	log << std::format("[{:%H:%M:%S}]", local) << " [" << caller << "/" << m_levelTexts[level] << "]: " << message << std::endl;

	std::cout << log.str();

	if (m_stream.is_open()) {
		m_stream << log.str();
		m_stream.flush();
	}
}
