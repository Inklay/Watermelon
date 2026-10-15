#include <string>
#define TOML_IMPLEMENTATION
#include "Config.h"
#include "Watermelon.h"
#include "Java/Packet/Packet.h"
#include "Java/Packet/Server/Parser.h"

#ifdef _WIN32
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#endif

Watermelon::Watermelon() :
	m_version({ "26.3", 777 }),
	m_config(loadConfig()),
	m_logger(*m_config["logging"].as_table()),
	m_running(false),
	m_registryManager(RegistryManager::getInstance()),
	m_entityCounter(0)
{
#ifdef _WIN32
	WSADATA wsaData;
	int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsaResult != 0) {
		log("WSA Straptup failed : " + std::to_string(wsaResult), Logger::ERR);
	}
#endif

	m_packs.emplace_back("minecraft", "core", m_version.name);
}

Watermelon::~Watermelon() {
	m_running = false;
#ifdef _WIN32
	if (m_serverSocket != INVALID_SOCKET) {
		closesocket(m_serverSocket);
	}
	WSACleanup();
#else
	if (m_serverSocket != -1) {
		close(m_serverSocket);
	}
#endif

	if (m_tcpListenThread.joinable()) {
		m_tcpListenThread.join();
	}
	if (m_networkThread.joinable()) {
		m_networkThread.join();
	}
}

bool Watermelon::start() {
	m_serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (!m_config["networking"]["java"]["enabled"].value_or(true)) {
		return false;
	}

	const std::string configuredAddress = m_config["networking"]["java"]["address"].value_or("0.0.0.0:25565");
	const int port = configuredAddress.find(':') != std::string::npos ? std::stoi(configuredAddress.substr(configuredAddress.find(':') + 1)) : 25565;
	const std::string address = configuredAddress.find(':') != std::string::npos ? configuredAddress.substr(0, configuredAddress.find(':')) : "0.0.0.0";
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	
	if (address == "0.0.0.0") {
		serverAddress.sin_addr.s_addr = INADDR_ANY;
	} else {
		inet_pton(AF_INET, address.c_str(), &serverAddress.sin_addr);
	}

	if (bind(m_serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
#ifdef _WIN32
		log("Bind error:" + std::to_string(WSAGetLastError()), Logger::DEBUG);
#else
		log("Bind error:" + strerror(errno), Logger::DEBUG);
#endif
		log("Can't listen to port " + std::to_string(port) + ", maybe it's already being used?", Logger::ERR);
		return false;
	}

	listen(m_serverSocket, 5);
	log("Listening on port TCP/" + std::to_string(port), Logger::INFO);
	m_running = true;
	m_tcpListenThread = std::thread(&Watermelon::tcpListenLoop, this);
	m_networkThread = std::thread(&Watermelon::networkLoop, this);

	return true;
}

void Watermelon::tcpListenLoop() {
	while (m_running) {
		sockaddr_in clientAddress;
 		int clientLength = sizeof(clientAddress);
		SOCKET clientSocket = accept(m_serverSocket, (struct sockaddr*)&clientAddress, &clientLength);

		if (clientSocket == -1) {
			if (!m_running) {
				break;
			}
#ifdef _WIN32
			log("Accept error:" + std::to_string(WSAGetLastError()), Logger::DEBUG);
#else
			log("Accept error:" + strerror(errno), Logger::DEBUG);
#endif
			continue;
		}

		log("New TCP connection", Logger::DEBUG);

		{
			std::lock_guard<std::mutex> lock(m_clientsMutex);
			m_clients.push_back(std::make_unique<Client>(Client::Type::JAVA, clientSocket));
		}
	}
	log("Stopping TCP listen thread", Logger::DEBUG);
}

void Watermelon::networkLoop() {
	while (m_running) {
		std::vector<std::shared_ptr<Client>> clientsToCheck;

		{
			std::lock_guard<std::mutex> lock(m_clientsMutex);
			clientsToCheck = m_clients;
		}

		if (clientsToCheck.empty()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			continue;
		}

		fd_set readFDs;
		FD_ZERO(&readFDs);
		SOCKET maxFD = 0;

		for (const auto& client : clientsToCheck) {
			FD_SET(client->getSocket(), &readFDs);
			if (client->getSocket() > maxFD) {
				maxFD = client->getSocket();
			}
		}

		struct timeval timeout { 0, 500000 };
		int activity = select(static_cast<int>(maxFD + 1), &readFDs, nullptr, nullptr, &timeout);

		if (activity == 0) {
			continue;
		}

		for (const auto& client : clientsToCheck) {
			if (FD_ISSET(client->getSocket(), &readFDs)) {
				if (client->getType() == Client::Type::JAVA) {
					Java::Packet::Header header = Java::Packet::Server::Parser::parseHeader(client);

					if (header.length == -1) {
						bool timedOut = false;

						{
							std::lock_guard<std::mutex> lock(m_clientsMutex);
							auto it = std::find(m_clients.begin(), m_clients.end(), client);
							if (it != m_clients.end()) {
								m_clients.erase(it);
							}
						}

						log("TCP connection closed", Logger::DEBUG);
						continue;
					}
					
					Java::Packet::Server::Parser::parsePacket(client, header, this);
				}
			}
		}
	}
	log("Stopping network thread", Logger::DEBUG);
}

void Watermelon::run() {
	while (m_running) {
		std::queue<std::pair<std::shared_ptr<Client>, std::function<void()>>> queuedPackets;
		{
			std::lock_guard<std::mutex> lock(m_packetQueueMutex);
			std::swap(m_packetQueue, queuedPackets);
		}

		while (!queuedPackets.empty()) {
			auto& [client, operation] = queuedPackets.front();
			operation();
			queuedPackets.pop();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}

void Watermelon::log(const std::string& message, Logger::Level level) {
	m_logger.log(message, "Watermelon", level);
}

std::mutex& Watermelon::getClientMutex() {
	return m_clientsMutex;
}

void Watermelon::addOperationToQueue(std::shared_ptr<Client> client, std::function<void()> packetHandler) {
	std::lock_guard<std::mutex> lock(m_packetQueueMutex);
	m_packetQueue.push(std::make_pair(client, packetHandler));
}

Watermelon::Version& Watermelon::getVersion() {
	return m_version;
}

Logger& Watermelon::getLogger() {
	return m_logger;
}

void Watermelon::sendDataToClient(std::shared_ptr<Client> client, const std::vector<char>& data) {
	std::vector<char> fullPacket;

	Java::Packet::writeVarInt((int32_t)data.size(), fullPacket);
	fullPacket.insert(fullPacket.end(), data.begin(), data.end());
	std::lock_guard<std::mutex> lock(m_clientsMutex);
	send(client->getSocket(), fullPacket.data(), (int)fullPacket.size(), 0);
}

toml::table& Watermelon::getConfig() {
	return m_config;
}

std::vector<Java::Pack>& Watermelon::getPacks() {
	return m_packs;
}

uint32_t Watermelon::increaseEntityCounter() {
	return ++m_entityCounter;
}
