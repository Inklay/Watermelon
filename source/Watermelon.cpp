#include <string>
#define TOML_IMPLEMENTATION
#include "Config.h"
#include "Watermelon.h"
#include "Java/Packet/Packet.h"
#include "Java/Packet/Server/Handler.h"

#ifdef _WIN32
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#endif

Watermelon::Watermelon() :
#ifdef _WIN32
	m_serverSocket(INVALID_SOCKET),
#else
	m_serverSocket(-1),
#endif
	m_version({ "26.3", 777 }),
	m_config(loadConfig()),
	m_logger(*m_config["logging"].as_table()),
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
	m_running.store(false);

	if (m_tcpListenThread.joinable()) {
		m_tcpListenThread.join();
	}
	if (m_networkThread.joinable()) {
		m_networkThread.join();
	}

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
}

bool Watermelon::start() {
	if (!m_config["networking"]["java"]["enabled"].value_or(true)) {
		return false;
	}

	m_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
#ifdef _WIN32
	if (m_serverSocket == INVALID_SOCKET) {
#else
	if (m_serverSocket == -1) {
#endif
		log("Can't create the server socket", Logger::ERR);
		return false;
	}

	const std::string configuredAddress = m_config["networking"]["java"]["address"].value_or("0.0.0.0:25565");
	int port = 0;
	try {
		port = configuredAddress.find(':') != std::string::npos ? std::stoi(configuredAddress.substr(configuredAddress.find(':') + 1)) : 25565;
		if (port < 0 || port > SHRT_MAX) {
			throw std::runtime_error("Invalid port number");
		}
	} catch (...) {
		log("Invalid port number: " + configuredAddress.substr(configuredAddress.find(':') + 1), Logger::ERR);
		return false;
	}
	const std::string address = configuredAddress.find(':') != std::string::npos ? configuredAddress.substr(0, configuredAddress.find(':')) : "0.0.0.0";
	sockaddr_in serverAddress{};
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	
	if (address == "0.0.0.0") {
		serverAddress.sin_addr.s_addr = INADDR_ANY;
	} else if (inet_pton(AF_INET, address.c_str(), &serverAddress.sin_addr)){
		log("Invalid listening address: " + address, Logger::ERR);
		return false;
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
	m_running.store(true);
	m_tcpListenThread = std::thread(&Watermelon::tcpListenLoop, this);
	m_networkThread = std::thread(&Watermelon::networkLoop, this);

	return true;
}

void Watermelon::tcpListenLoop() {
	while (m_running.load()) {
		fd_set readFDs;
		FD_ZERO(&readFDs);
		FD_SET(m_serverSocket, &readFDs);

		struct timeval timeout { 0, 500000 };
		int activity = select(static_cast<int>(m_serverSocket + 1), &readFDs, nullptr, nullptr, &timeout);

		if (activity <= 0) {
			continue;
		}

		sockaddr_in clientAddress;
 		int clientLength = sizeof(clientAddress);
		SOCKET clientSocket = accept(m_serverSocket, (struct sockaddr*)&clientAddress, &clientLength);

		if (clientSocket == -1) {
			if (!m_running.load()) {
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
			m_clients.push_back(std::make_shared<Client>(Client::Type::JAVA, clientSocket, this));
		}
	}
	m_logger.log("Stopping TCP listen thread", "TCP listen", Logger::INFO);
}

void Watermelon::networkLoop() {
	while (m_running.load()) {
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

		if (activity <= 0) {
			continue;
		}

		for (auto& client : clientsToCheck) {
			if (!FD_ISSET(client->getSocket(), &readFDs)) {
				continue;
			}

			char received[8192];
			int count = static_cast<int>(recv(client->getSocket(), received, sizeof(received), 0));

			if (count <= 0) {
				disconnectClient(client.get());
				continue;
			}

			client->appendReceivedData(reinterpret_cast<const uint8_t*>(received), (size_t)count);

			while (true) {
				std::vector<uint8_t> packet;
				const auto result = client->readPacket(packet);

				if (result == Client::PacketStatus::INCOMPLETE) {
					break;
				}

				if (result == Client::PacketStatus::INVALID) {
					disconnectClient(client.get());
					break;
				}

				if (client->getType() == Client::Type::JAVA) {
					try {
						Java::Packet::Server::Handler::handlePacket(client, this, packet);
					} catch (const std::exception& e) {
						log(std::string("Exception while handling a packet: ") + e.what(), Logger::ERR);
						disconnectClient(client.get());
						break;
					}
				} else {
					break;
				}
			}
		}
	}
	m_logger.log("Stopping network thread", "Network", Logger::INFO);
}

void Watermelon::run() {
	while (m_running.load()) {
		std::queue<std::pair<std::shared_ptr<Client>, std::function<void()>>> queuedPackets;
		{
			std::lock_guard<std::mutex> lock(m_packetQueueMutex);
			std::swap(m_packetQueue, queuedPackets);
		}

		while (!queuedPackets.empty()) {
			auto& [client, operation] = queuedPackets.front();

			try {
				operation();
			} catch (const std::exception& e) {
				log(std::string("Exception while handling a client operation: ") + e.what(), Logger::ERR);
				disconnectClient(client.get());
			} catch (...) {
				log("Unknown exception while handling a client operation", Logger::ERR);
				disconnectClient(client.get());
			}

			queuedPackets.pop();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	log("Stopping main thread", Logger::INFO);
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
	client->sendData(fullPacket);
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

void Watermelon::disconnectClient(Client* client) {
	std::shared_ptr<Client> ptr;
	{
		std::lock_guard<std::mutex> lock(m_clientsMutex);
		auto it = std::find_if(m_clients.begin(), m_clients.end(),
			[client](const std::shared_ptr<Client>& p) {
				return p.get() == client;
			}
		);

		if (it != m_clients.end()) {
			ptr = *it;
			m_clients.erase(it);
		}
	}

	if (ptr) {
		ptr->disconnect();
		log("TCP connection closed", Logger::DEBUG);
	}
}
