#include <sstream>
#include <chrono>
#include <iostream>
#include <format>
#include <thread>
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
	if (!m_config["enabled"].value_or(true) || m_config["level"].value_or<std::string>("info") == "off") {
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

	if (m_config["timestamp"].value_or(true)) {
		std::string format = m_config["timestamp_format"].value_or<std::string>("[hour]:[minute]:[second]");
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

		if (m_config["color"].value_or(true)) {
			log << "\033[32m";
		}

		log << std::vformat("[{:" + format + "}]", std::make_format_args(local));
	}

	if (m_config["color"].value_or(true)) {
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
		}
	}

	log << " [";

	if (m_config["threads"].value_or(true)) {
		log << caller << "/";
	}

	if (m_config["thread_ids"].value_or(false)) {
		log << std::this_thread::get_id() << "/";
	}

	log << m_levelTexts[level] << "]\033[0m: " << message << std::endl;

	std::lock_guard<std::mutex> lock(m_mutex);
	std::cout << log.str();

	if (m_stream.is_open()) {
		m_stream << log.str();
		m_stream.flush();
	}
}
