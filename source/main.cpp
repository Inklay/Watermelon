#include <csignal>
#include "Core/Watermelon.h"

namespace {
	static_assert(std::atomic<Watermelon*>::is_always_lock_free);
	static_assert(std::atomic<bool>::is_always_lock_free);

	std::atomic<Watermelon*> g_server { nullptr };
	std::atomic<bool> g_stopRequested { false };

	void handleShutdownSignal(int) {
		if (g_stopRequested.exchange(true)) {
			std::_Exit(1);
		}

		if (Watermelon* server = g_server.load()) {
			server->stop();
		}
	}
}

int main()
{
	Watermelon server;
	if (server.start()) {
		g_server.store(&server);
		std::signal(SIGINT, handleShutdownSignal);
		std::signal(SIGTERM, handleShutdownSignal);

		server.run();

		g_server.store(nullptr);
		server.getLogger().log("Shutting down...", "Watermelon", Logger::INFO);
	}

	return 0;
}
