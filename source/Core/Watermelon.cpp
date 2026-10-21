#include <string>
#include "Core/Config.h"
#include "Core/Watermelon.h"
#include "Java/Packet/Packet.h"
#include "Java/Packet/Server/Handler.h"
#include "Utils/Base64.h"
#include <nlohmann/json.hpp>
#include "Utils/UUID.h"
#include "Auth/Auth.h"
#include "Java/Packet/Client/LoginPackets.h"
#ifdef _WIN32
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <csignal>
#include <netinet/tcp.h>
#endif

Watermelon::Watermelon() :
#ifdef _WIN32
	m_serverSocket(INVALID_SOCKET),
#else
	m_serverSocket(-1),
#endif
	m_config(loadConfig()),
	m_version({ "26.3", 777 }),
	m_logger(*m_config["logging"].as_table()),
	m_registryManager(RegistryManager::getInstance()),
	m_entityCounter(0),
	m_defaultGamemode(Player::Gamemode::CREATIVE),
	m_keys(m_logger, m_config["networking"]["java"]["online_mode"].value_or(true)),
	m_authPool(4, 64, "Worker/Auth", m_logger)
{
#ifdef _WIN32
	WSADATA wsaData;
	int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsaResult != 0) {
		log("WSA Straptup failed : " + std::to_string(wsaResult), Logger::ERR);
	}
#else
	signal(SIGPIPE, SIG_IGN);
#endif

	m_packs.emplace_back("minecraft", "core", m_version.name);

	if (m_config["default_gamemode"].value_or<std::string>("Survival") == "Creative") {
		m_defaultGamemode = Player::Gamemode::CREATIVE;
	} else if (m_config["default_gamemode"].value_or<std::string>("Survival") == "Spectator") {
		m_defaultGamemode = Player::Gamemode::SPECTATOR;
	} else if (m_config["default_gamemode"].value_or<std::string>("Survival") == "Adventure") {
		m_defaultGamemode = Player::Gamemode::ADVENTURE;
	} else {
		m_defaultGamemode = Player::Gamemode::SURVIVAL;
	}

	if (m_config["use_favicon"].value_or(true)) {
		readFavicon();
	}
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

#ifndef _WIN32
	int yes = 1;
	setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
#endif

	const std::string configuredAddress = m_config["networking"]["java"]["address"].value_or("0.0.0.0:25565");
	int port = 0;
	try {
		port = configuredAddress.find(':') != std::string::npos ? std::stoi(configuredAddress.substr(configuredAddress.find(':') + 1)) : 25565;
		if (port < 1 || port > 65535) {
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
	} else if (inet_pton(AF_INET, address.c_str(), &serverAddress.sin_addr) != 1){
		log("Invalid listening address: " + address, Logger::ERR);
		return false;
	}

	if (bind(m_serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
#ifdef _WIN32
		log("Bind error: " + std::to_string(WSAGetLastError()), Logger::DEBUG);
#else
		log(std::string("Bind error: ") + strerror(errno), Logger::DEBUG);
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
#ifdef _WIN32
 		int clientLength = sizeof(clientAddress);
#else
 		unsigned int clientLength = sizeof(clientAddress);
#endif
		Socket clientSocket = accept(m_serverSocket, (struct sockaddr*)&clientAddress, &clientLength);

		if (clientSocket == -1) {
			if (!m_running.load()) {
				break;
			}
#ifdef _WIN32
			log("Accept error: " + std::to_string(WSAGetLastError()), Logger::DEBUG);
#else
			log(std::string("Accept error : ") + strerror(errno), Logger::DEBUG);
#endif
			continue;
		}

		int yes = 1;
		setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&yes), sizeof(yes));

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
		Socket maxFD = 0;

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

			client->appendReceivedData();

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
	int configTps = m_config["tps"].value_or(20);

	if (configTps <= 0) {
		configTps = 20;
		log("Configured TPS must be greater than 0, using default value", Logger::WARN);
	}

	const auto tickDuration = std::chrono::milliseconds(1000 / configTps);

	std::chrono::steady_clock::time_point nextTick = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point nextKeepAlive = std::chrono::steady_clock::now() + std::chrono::seconds(10);

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

		std::vector<std::shared_ptr<Client>> clients;
		{
			std::lock_guard<std::mutex> lock(m_clientsMutex);
			clients = m_clients;
		}

		std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
		bool sendKeepAlive = now > nextKeepAlive;

		for (const std::shared_ptr<Client>& client : clients) {
			if (client->hasPlayer() && client->getStatus() == Client::Status::PLAY) {
				Player::Player* player = client->getPlayer();

				if (player->isLoaded()) {
					if (sendKeepAlive) {
						try {
							client->keepAlive(now);
						} catch (...) {
							log("Failed to send keep alive", Logger::WARN);
						}
					}

					player->tickEnd();
				}
			}
		}

		if (sendKeepAlive) {
			log("Sent keep alive", Logger::DEBUG);
			nextKeepAlive += std::chrono::seconds(10);
		}

		nextTick += tickDuration;
		now = std::chrono::steady_clock::now();

		if (now < nextTick) {
			std::this_thread::sleep_until(nextTick);
		} else {
			nextTick = now;
		}
	}

	log("Stopping main thread", Logger::INFO);
}

void Watermelon::log(const std::string& message, Logger::Level level) {
	m_logger.log(message, "Watermelon", level);
}

std::vector<const Player::Player*> Watermelon::getPlayers() {
	std::vector<const Player::Player*> players;
	std::lock_guard<std::mutex> lock(m_clientsMutex);

	for (const auto& client : m_clients) {
		if (client->hasPlayer()) {
			players.emplace_back(client->getPlayer());
		}
	}

	return players;
}

std::vector<std::shared_ptr<Client>> Watermelon::getClients() {
	std::lock_guard<std::mutex> lock(m_clientsMutex);
	return m_clients;
}

void Watermelon::addOperationToQueue(const std::shared_ptr<Client>& client, const std::function<void()>& packetHandler) {
	std::lock_guard<std::mutex> lock(m_packetQueueMutex);
	m_packetQueue.push(std::make_pair(client, packetHandler));
}

Watermelon::Version& Watermelon::getVersion() {
	return m_version;
}

Logger& Watermelon::getLogger() {
	return m_logger;
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
			if (it->get()->hasPlayer()) {
				m_playerCount--;
			}
			ptr = *it;
			m_clients.erase(it);
		}
	}

	if (ptr) {
		ptr->disconnect();
		log("TCP connection closed", Logger::DEBUG);
	}

	updatePlayerSample();
}

int Watermelon::getPlayerCount() {
	return m_playerCount;
}

void Watermelon::increasePlayerCount() {
	m_playerCount++;
}

Player::Gamemode Watermelon::getDefaultGamemode() {
	return m_defaultGamemode;
}

void Watermelon::readFavicon() {
	constexpr size_t MAX_FILE_SIZE = 1000000;
	const std::string path = m_config["favicon_path"].value_or("icon.png");
	std::ifstream file(path, std::ios::binary);

	if (!file) {
		log("Can't open favicon file " + path, Logger::WARN);
		return;
	}

	file.seekg(0, std::ios::end);
	const auto fileSize = file.tellg();

	if (fileSize <= 0 ||
		static_cast<uint64_t>(fileSize) > MAX_FILE_SIZE) {
		log("Favicon file is empty or too large: " + path, Logger::WARN);
		return;
	}

	file.seekg(0);
	std::vector<uint8_t> data(static_cast<size_t>(fileSize));
	static constexpr uint8_t PNG_MAGIC[8] = { 0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n' };

	if (!file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(data.size()))) {
		log("Failed to read favicon: " + path, Logger::WARN);
		return;
	}

	if (data.size() < 24 || !std::equal(std::begin(PNG_MAGIC), std::end(PNG_MAGIC), data.begin())) {
		log("Favicon: " + path + " is not a valid PNG file", Logger::WARN);
		return;
	}

	const uint32_t width = (data[16] << 24) | (data[17] << 16) | (data[18] << 8) | data[19];
	const uint32_t height = (data[20] << 24) | (data[21] << 16) | (data[22] << 8) | data[23];

	if (width != 64 || height != 64) {
		log("Favicon must be exactly 64x64 pixels: " + path, Logger::WARN);
		return;
	}

	m_faviconBase64 = ("data:image/png;base64," + Utils::Base64::encode(data));
}

const std::string& Watermelon::getFaviconData() {
	return m_faviconBase64;
}

void Watermelon::stop() {
	m_running.store(false);
}

Keys& Watermelon::getKeys() {
	return m_keys;
}

void Watermelon::updatePlayerSample() {
	std::lock_guard<std::mutex> lock(m_clientsMutex);

	if (!m_playerSample.empty()) {
		m_playerSample.clear();
	}

	for (std::shared_ptr<Client>& client : m_clients) {
		if (client->hasPlayer()) {
			Player::Player* player = client->getPlayer();

			if (player->getClientInfo().shownOnServerList) {
				m_playerSample.push_back(std::make_pair<const std::string&, std::pair<uint64_t, uint64_t>>(player->getUsername(), player->getUUID()));

				if (m_playerSample.size() >= 5) {
					return;
				}
			}
		}
	}
}

std::vector<std::pair<const std::string, std::pair<uint64_t, uint64_t>>> Watermelon::getPlayerSample() {
	std::lock_guard<std::mutex> lock(m_clientsMutex);
	return m_playerSample;
}

bool Watermelon::isOnlineMode() { 
	return m_config["networking"]["java"]["online_mode"].value_or(true);
}

bool Watermelon::isEncryptionEnabled() { 
	return isOnlineMode() || m_config["networking"]["java"]["encryption"].value_or(true);
}

void Watermelon::addAuthTask(const std::shared_ptr<Client>& client) {
	bool taskResult =  m_authPool.submit([client, server = this]() {
		Auth::Result result = Auth::authenticatePlayer(client, server);
		server->addOperationToQueue(client, [client, server, result = std::move(result)]() {
			switch (result.status) {
			case Auth::Status::FAILED:
				client->startEncryption();
				return Java::Packet::Client::Login::disconnect(client, server, nlohmann::json({{ "translate", "multiplayer.disconnect.unverified_username" }}).dump());
			case Auth::Status::SERVER_DOWN:
				client->startEncryption();
				return Java::Packet::Client::Login::disconnect(client, server, nlohmann::json({ { "translate", "multiplayer.disconnect.authservers_down" } }).dump());
			case Auth::Status::AUTHENTIFIED:
				client->startEncryption();
				try {
					std::pair<uint64_t, uint64_t> uuid = Utils::UUID::fromString(result.uuid);
					if (!Player::isUsernameValid(result.username)) {
						throw std::runtime_error("Invalid username");
					}

					client->createPlayer(result.username, uuid, server->increaseEntityCounter(), { 0, 0, 0 }, { 0, 0 });
					client->getPlayer()->setProfileProperties(result.properties);
				} catch (...) {
					return Java::Packet::Client::Login::disconnect(client, server, nlohmann::json({ { "translate", "multiplayer.disconnect.unverified_username" } }).dump());;
				}

				Java::Packet::Client::Login::setCompression(client);
				Java::Packet::Client::Login::loginSuccess(client);
			}
		});
	});

	if (!taskResult) {
		log("Auth pool are busy", Logger::WARN);
		disconnectClient(client.get());
	}
}
