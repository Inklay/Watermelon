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

class Watermelon {
public:
	struct Version {
		std::string name;
		int protocol;
	};

	Watermelon(const std::string& address, uint16_t port);
	~Watermelon();
	bool start();
	void run();
	std::mutex& getClientMutex();
	int getJavaProtocolVersion();
	void addPacketToQueue(std::shared_ptr<Client> client, std::function<std::vector<char>()> packetHandler);
	Version getVersion();
	Logger& getLogger();
	void sendDataToClient(std::shared_ptr<Client> client, const std::vector<char>& data);

private:
	void log(const std::string& message, Logger::Level level);
	void tcpListenLoop();
	void networkLoop();

	const std::string m_address;
	Socket m_serverSocket;
	Logger m_logger;
	std::thread m_tcpListenThread;
	std::thread m_networkThread;
	std::vector<std::shared_ptr<Client>> m_clients;
	std::mutex m_clientsMutex;
	std::mutex m_packetQueueMutex;
	std::queue<std::pair<std::shared_ptr<Client>, std::function<std::vector<char>()>>> m_packetQueue;
	Version m_version;
	const uint16_t m_port;
	bool m_running;
};
