#pragma once

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <memory>
#include <queue>
#include <functional>
#include "Logger.h"
#include "Client.h"
#define TOML_IMPLEMENTATION
#include "Utils/TOML.h"
#include "Registry/RegistryManager.h"

class Watermelon {
public:
	struct Version {
		std::string name;
		int protocol;
	};

	Watermelon();
	~Watermelon();
	bool start();
	void run();
	std::mutex& getClientMutex();
	void addPacketToQueue(std::shared_ptr<Client> client, std::function<void()> packetHandler);
	Version& getVersion();
	Logger& getLogger();
	void sendDataToClient(std::shared_ptr<Client> client, const std::vector<char>& data);
	toml::table& getConfig();
	std::vector<Java::Pack>& getPacks();
	uint32_t increaseEntityCounter();

private:
	void log(const std::string& message, Logger::Level level);
	void tcpListenLoop();
	void networkLoop();

	Socket m_serverSocket;
	toml::table m_config;
	Logger m_logger;
	std::thread m_tcpListenThread;
	std::thread m_networkThread;
	std::vector<std::shared_ptr<Client>> m_clients;
	std::mutex m_clientsMutex;
	std::mutex m_packetQueueMutex;
	std::queue<std::pair<std::shared_ptr<Client>, std::function<void()>>> m_packetQueue;
	std::vector<Java::Pack> m_packs;
	RegistryManager& m_registryManager;
	Version m_version;
	uint32_t m_entityCounter;
	bool m_running;
};
