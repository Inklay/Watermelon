#include <sstream>
#include <chrono>
#include <iostream>
#include <format>
#include "Logger.h"

Logger::Logger(const std::string& path) :
	m_stream(path, std::ios::out | std::ios::trunc)
{
	if (m_stream.bad()) {
		log("Can't create log file '" + path + "'", "Watermelon", ERR);
	}
}

Logger::~Logger() {
	m_stream.close();
}

void Logger::log(const std::string& message, const std::string& caller, Level level) {
	if (level > ERR) {
		return;
	}

#ifdef _DEBUG
	if (level < DEBUG) {
		return;
	}
#else
	if (level < INFO) {
		return;
	}
#endif

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
