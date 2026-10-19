#include "Core/TaskPool.h"

TaskPool::TaskPool(size_t threads, size_t maxQueued, const std::string& name, Logger& logger) :
	m_maxQueued(maxQueued),
	m_name(name),
	m_logger(logger)
{
	for (size_t i = 0; i < threads; i++) {
		m_threads.emplace_back(&TaskPool::worker, this);
	}
}

TaskPool::~TaskPool() {
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_stopping = true;
	}

	m_condition.notify_all();

	for (std::thread& thread : m_threads) {
		thread.join();
	}
}

bool TaskPool::submit(std::function<void()> task) {
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		if (m_stopping || m_tasks.size() >= m_maxQueued) {
			return false;
		}

		m_tasks.push(std::move(task));
	}

	m_condition.notify_one();
	return true;
}

void TaskPool::worker() {
	while (true) {
		std::function<void()> task;

		{
			std::unique_lock<std::mutex> lock(m_mutex);
			m_condition.wait(lock, [this] { 
				return m_stopping || !m_tasks.empty();
			});

			if (m_stopping) {
				return;
			}

			task = std::move(m_tasks.front());
			m_tasks.pop();
		}

		try {
			task();
		} catch (...) {
			m_logger.log("An error occuried on a worker thread", m_name, Logger::WARN);
		}
	}
}
