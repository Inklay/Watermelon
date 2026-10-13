#include <string>
#include "Watermelon.h"
#include "Java/Packet.h"
#include "Java/Parser/Parser.h"

#ifndef _WIN32
#include <sys/socket.h>
#endif

Watermelon::Watermelon(const std::string& address, uint16_t port) :
	m_address(address),
	m_logger("latest.log"),
	m_version({ "Watermelon 26.3", 777 }),
	m_port(port),
	m_running(false)
{
#ifdef _WIN32
	WSADATA wsaData;
	int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsaResult != 0) {
		log("WSA Straptup failed : " + std::to_string(wsaResult), Logger::ERR);
	}
#endif
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

	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(m_port);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	if (bind(m_serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
#ifdef _WIN32
		log("Bind error:" + std::to_string(WSAGetLastError()), Logger::DEBUG);
#else
		log("Bind error:" + strerror(errno), Logger::DEBUG);
#endif
		log("Can't listen to port " + std::to_string(m_port) + ", maybe it's already being used?", Logger::ERR);
		return false;
	}

	listen(m_serverSocket, 5);
	log("listening on port " + std::to_string(m_port), Logger::INFO);
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

		log("New TCP connection", Logger::INFO);

		{
			std::lock_guard<std::mutex> lock(m_clientsMutex);
			m_clients.push_back(std::make_unique<Client>(Client::Type::JAVA, clientSocket, Client::Status::HANDSHAKING));
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
			FD_SET(client->socket, &readFDs);
			if (client->socket > maxFD) {
				maxFD = client->socket;
			}
		}

		struct timeval timeout { 0, 500000 };
		int activity = select(static_cast<int>(maxFD + 1), &readFDs, nullptr, nullptr, &timeout);

		if (activity == 0) {
			continue;
		}

		for (const auto& client : clientsToCheck) {
			if (FD_ISSET(client->socket, &readFDs)) {
				if (client->type == Client::Type::JAVA) {
					Java::Packet::Header header = Java::Parser::parseHeader(client);

					if (header.length == -1) {
						bool timedOut = false;

						{
							std::lock_guard<std::mutex> lock(m_clientsMutex);
#ifdef _WIN32
							closesocket(client->socket);
#else
							close(client->socket);
#endif
							auto it = std::find(m_clients.begin(), m_clients.end(), client);
							if (it != m_clients.end()) {
								m_clients.erase(it);
							}
						}

						log("TCP connection closed", Logger::DEBUG);
						continue;
					}
					
					Java::Parser::parsePacket(client, header, this);
				}
			}
		}
	}
	log("Stopping network thread", Logger::DEBUG);
}

void Watermelon::run() {
	while (m_running) {
		std::queue<std::pair<std::shared_ptr<Client>, std::function<std::vector<char>()>>> queuedPackets;
		{
			std::lock_guard<std::mutex> lock(m_packetQueueMutex);
			std::swap(m_packetQueue, queuedPackets);
		}

		while (!queuedPackets.empty()) {
			auto& [client, packetHandler] = queuedPackets.front();
			std::vector<char> response = packetHandler();
			queuedPackets.pop();

			if (!response.empty()) {
				sendDataToClient(client, response);
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}

void Watermelon::log(const std::string& message, Logger::Level level) {
	m_logger.log(message, "Watermelon", level);
}

int Watermelon::getJavaProtocolVersion() {
	return 777;
}

std::mutex& Watermelon::getClientMutex() {
	return m_clientsMutex;
}

void Watermelon::addPacketToQueue(std::shared_ptr<Client> client, std::function<std::vector<char>()> packetHandler) {
	std::lock_guard<std::mutex> lock(m_packetQueueMutex);
	m_packetQueue.push(std::make_pair(client, packetHandler));
}

Watermelon::Version Watermelon::getVersion() {
	return m_version;
}

Logger& Watermelon::getLogger() {
	return m_logger;
}

void Watermelon::sendDataToClient(std::shared_ptr<Client> client, const std::vector<char>& data) {
	std::vector<char> fullPacket;

	Java::Packet::writeVarInt(client, (int32_t)data.size(), fullPacket);
	fullPacket.insert(fullPacket.end(), data.begin(), data.end());
	std::lock_guard<std::mutex> lock(m_clientsMutex);
	send(client->socket, fullPacket.data(), (int)fullPacket.size(), 0);
}
