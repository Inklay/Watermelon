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
#include "Player/Player.h"
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
	std::vector<const Player*> getPlayers();
	std::vector<std::shared_ptr<Client>> getClients();
	void addOperationToQueue(std::shared_ptr<Client> client, std::function<void()> packetHandler);
	Version& getVersion();
	Logger& getLogger();
	void sendDataToClient(std::shared_ptr<Client> client, const std::vector<char>& data);
	toml::table& getConfig();
	std::vector<Java::Pack>& getPacks();
	uint32_t increaseEntityCounter();
	void disconnectClient(Client* client);
	int getPlayerCount();
	void increasePlayerCount();
	Player::Gamemode getDefaultGamemode();
	const std::string& getFaviconData();

private:
	void log(const std::string& message, Logger::Level level);
	void tcpListenLoop();
	void networkLoop();
	void readFavicon();

	Socket m_serverSocket;
	toml::table m_config;
	Version m_version;
	Logger m_logger;
	std::thread m_tcpListenThread;
	std::thread m_networkThread;
	std::vector<std::shared_ptr<Client>> m_clients;
	std::mutex m_clientsMutex;
	std::mutex m_packetQueueMutex;
	std::queue<std::pair<std::shared_ptr<Client>, std::function<void()>>> m_packetQueue;
	std::vector<Java::Pack> m_packs;
	RegistryManager& m_registryManager;
	std::string m_faviconBase64;
	uint32_t m_entityCounter;
	std::atomic<bool> m_running { false };
	std::atomic<int> m_playerCount { 0 };
	Player::Gamemode m_defaultGamemode;
};
