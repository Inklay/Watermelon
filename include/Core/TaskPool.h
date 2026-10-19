#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include "Core/Logger.h"

class TaskPool {
public:
	TaskPool(size_t threads, size_t maxQueued, const std::string& name, Logger& logger);
	~TaskPool();
	TaskPool(const TaskPool&) = delete;
	TaskPool(const TaskPool&&) = delete;
	TaskPool& operator=(const TaskPool&) = delete;
	TaskPool& operator=(const TaskPool&&) = delete;
	bool submit(std::function<void()> task);

private:
	void worker();
	void log(const std::string& message);

	const size_t m_maxQueued;
	bool m_stopping = false;
	std::mutex m_mutex;
	std::condition_variable m_condition;
	std::queue<std::function<void()>> m_tasks;
	std::vector<std::thread> m_threads;
	const std::string m_name;
	Logger& m_logger;
};
