#include "Core/Client.h"
#include "Core/Watermelon.h"
#include "Crypto/Crypto.h"
#include "Utils/UUID.h"
#include "Java/Packet/Packet.h"
#include "Compression/ZLIB.h"
#ifndef _WIN32
#include <unistd.h>
#include <sys/socket.h>
#endif

Client::Client(Type type, Socket socket, Watermelon* server) :
	m_type(type),
	m_status(Status::HANDSHAKING),
	m_socket(socket),
	m_player(nullptr),
	m_server(server),
	m_disconnected(false)
{
}

Client::~Client() {
	if (m_player != nullptr) {
		delete m_player;
	}

#ifdef _WIN32
	closesocket(m_socket);
#else
	close(m_socket);
#endif
}

Socket Client::getSocket() const {
	return m_socket;
}

Client::Type Client::getType() const {
	return m_type;
}

bool Client::hasPack(const std::string& name) {
	std::lock_guard<std::mutex> lock(m_mutex);
	for (const auto& pack : m_packs) {
		if (pack.nameSpace == name) {
			return true;
		}
	}

	return false;
}

void Client::changeStatus(Status status) {
	std::lock_guard<std::mutex> lock(m_mutex);
	m_status = status;
}

Client::Status Client::getStatus() {
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_status;
}

bool Client::startLogin(const std::string& username) {
	bool disconnect = false;
	std::string logMessage;

	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_player != nullptr || !m_tempPlayerName.empty()) {
			disconnect = true;
			logMessage = "Player was already logged in";
		} else if (!Player::isUsernameValid(username)) {
			disconnect = true;
			logMessage = "Invalid player name";
		}

		m_tempPlayerName = username;
	}

	if (disconnect) {
		m_server->getLogger().log(logMessage, "Network", Logger::WARN);
		m_server->disconnectClient(this);
		return false;
	}

	return true;
}

void Client::createPlayer(const std::string& username, std::pair<uint64_t, uint64_t> uuid, int32_t entityId, Entity::Position position, Entity::Rotation rotation) {
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_tempPlayerName = username;
	}

	createPlayer(uuid, entityId, position, rotation);
}

void Client::createPlayer(std::pair<uint64_t, uint64_t> uuid, int32_t entityId, Entity::Position position, Entity::Rotation rotation) {	
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_player = new Player::Player(m_tempPlayerName, uuid, entityId, position, rotation, m_server);
	}

	m_server->getLogger().log("UUID of player " + m_tempPlayerName + " is " + Utils::UUID::formatUUID(uuid), "Network", Logger::INFO);
	m_server->increasePlayerCount();
}

Player::Player* Client::getPlayer() {
	std::lock_guard<std::mutex> lock(m_mutex);

	if (m_player == nullptr) {
		throw std::runtime_error("Accessing data of a non existant player");
	}

	return m_player;
}

bool Client::hasPlayer() {
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_player != nullptr;
}

void Client::setPacks(const std::vector<Java::Pack> packs) {
	std::lock_guard<std::mutex> lock(m_mutex);
	m_packs = packs;
}

std::vector<Java::Pack> Client::getPacks() {
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_packs;
}

void Client::setSessionUUID(std::pair<uint64_t, uint64_t> sessionUUID) {
	std::lock_guard<std::mutex> lock(m_mutex);
	m_sessionUUID = sessionUUID;
}

Client::PacketStatus Client::readPacket(std::vector<uint8_t>& packet) {
	std::lock_guard<std::mutex> lock(m_mutex);

	if (m_disconnected) {
		return Client::PacketStatus::INVALID;
	}

	uint32_t length = 0;
	size_t prefixLength = 0;

	for (int i = 0; i < 3; ++i) {
		if (prefixLength >= m_receiveBuffer.size()) {
			return Client::PacketStatus::INCOMPLETE;
		}

		const uint8_t byte = m_receiveBuffer[prefixLength++];
		length |= uint32_t(byte & 0x7F) << (7 * i);

		if ((byte & 0x80) == 0) {
			break;
		}

		if (i == 2) {
			return Client::PacketStatus::INVALID;
		}
	}

	constexpr uint32_t MAX_PACKET_SIZE = 0x1FFFFF;

	if (length == 0 || length > MAX_PACKET_SIZE) {
		return Client::PacketStatus::INVALID;
	}

	if (m_receiveBuffer.size() - prefixLength < length) {
		return Client::PacketStatus::INCOMPLETE;
	}

	if (m_compressionThreshold.load() != -1) {
		std::vector<uint8_t> compressedPacket(m_receiveBuffer.begin() + prefixLength, m_receiveBuffer.begin() + prefixLength + length);
		uint32_t offset = 0;

		try {
			uint32_t dataLenght = Java::Packet::readVarInt(compressedPacket, offset);
			compressedPacket.erase(compressedPacket.begin(), compressedPacket.begin() + offset);

			if (dataLenght == 0) {
				if (static_cast<int32_t>(length) >= m_compressionThreshold.load()) {
					return Client::PacketStatus::INVALID;
				}

				packet = compressedPacket;
			} else if (dataLenght < MAX_PACKET_SIZE && static_cast<int32_t>(dataLenght) >= m_compressionThreshold.load()) {
				std::vector<uint8_t> decompressedData = Compression::ZLIB::decompress(compressedPacket, dataLenght);

				if (decompressedData.size() != dataLenght) {
					return Client::PacketStatus::INVALID;
				}
				packet = decompressedData;
			} else {
				return Client::PacketStatus::INVALID;
			}
		} catch (...) {
			return Client::PacketStatus::INVALID;
		}
	} else {
		packet.assign(m_receiveBuffer.begin() + prefixLength, m_receiveBuffer.begin() + prefixLength + length);
	}

	m_receiveBuffer.erase(m_receiveBuffer.begin(), m_receiveBuffer.begin() + prefixLength + length);
	return Client::PacketStatus::COMPLETE;
}

void Client::appendReceivedData() {
	constexpr int32_t MAX_BUFFER_SIZE = 4000000;
	bool disconnect = false;
	char received[8192];
	int count = static_cast<int>(recv(m_socket, received, sizeof(received), 0));

	if (count <= 0) {
		m_server->disconnectClient(this);
		return;
	}

	std::vector<uint8_t> incoming(reinterpret_cast<const uint8_t*>(received), reinterpret_cast<const uint8_t*>(received) + count);

	if (m_encrypted.load()) {
		incoming = m_cipher.decrypt(incoming);
	}

	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_receiveBuffer.size() + count > MAX_BUFFER_SIZE) {
			disconnect = true;
		} else {
			m_receiveBuffer.insert(m_receiveBuffer.end(), incoming.begin(), incoming.end());
		}
	}

	if (disconnect) {
		m_server->getLogger().log("Client exceded max buffer size", "Network", Logger::WARN);
		m_server->disconnectClient(this);
	}
}

void Client::disconnect() {
	std::lock_guard<std::mutex> lock(m_mutex);
	m_disconnected = true;
}

void Client::sendData(std::vector<uint8_t>& data) {
	bool disconnect = false;

	{
		std::lock_guard<std::mutex> lock(m_sendingMutex);
		size_t totalSent = 0;
		

		if (m_compressionThreshold.load() != -1) {
			std::vector<uint8_t> compressedPacket;

			if (static_cast<int>(data.size()) >= m_compressionThreshold) {
				try {
					std::vector<uint8_t> compressedData = Compression::ZLIB::compress(data);
					Java::Packet::writeVarInt(static_cast<uint32_t>(data.size()), compressedPacket);
					compressedPacket.insert(compressedPacket.end(), compressedData.begin(), compressedData.end());
				} catch (...) {
					m_server->disconnectClient(this);
					return;
				}
			} else {
				Java::Packet::writeVarInt(0, compressedPacket);
				compressedPacket.insert(compressedPacket.end(), data.begin(), data.end());
			}

			data = compressedPacket;
		}

		std::vector<uint8_t> fullPacket;
		Java::Packet::writeVarInt((int32_t)data.size(), fullPacket);
		fullPacket.insert(fullPacket.end(), data.begin(), data.end());
		size_t size = fullPacket.size();

		if (m_encrypted.load()) {
			fullPacket = m_cipher.encrypt(fullPacket);
		}

		while (totalSent < size) {
			const size_t remaining = size - totalSent;
			const int toSend = static_cast<int>((std::min)(remaining, static_cast<size_t>(INT_MAX)));
			const int sent = send(m_socket, reinterpret_cast<const char*>(fullPacket.data()) + totalSent, toSend, 0);

			if (sent == -1) {
				disconnect = true;
				break;
			}

			if (sent == 0) {
				return;
			}

			totalSent += static_cast<size_t>(sent);
		}
	}

	if (disconnect) {
		m_server->disconnectClient(this);
	}
}

bool Client::isEncrypted() {
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_encrypted.load();
}

void Client::startEncryption() {
	std::lock_guard<std::mutex> lock(m_mutex);

	if (m_encrypted.load()) {
		throw std::runtime_error("Client already ecnrypted");
	}

	m_cipher.init(m_secret);
	m_encrypted.store(true);
}

const std::vector<uint8_t>& Client::generateEncryptionToken() {
	std::lock_guard<std::mutex> lock(m_mutex);
	m_encryptionToken = Crypto::generateRandomBytes(8);
	return m_encryptionToken;
}

bool Client::checkToken(const std::vector<uint8_t>& token) {
	std::lock_guard<std::mutex> lock(m_mutex);

	if (token.size() != m_encryptionToken.size() || m_encryptionToken.empty()) {
		return false;
	}

	for (int i = 0; i < (int)m_encryptionToken.size(); i++) {
		if (m_encryptionToken.at(i) != token.at(i)) {
			return false;
		}
	}

	return true;
}

std::string Client::getTempUsername() {
	return m_tempPlayerName;
}

bool Client::authenticate(const std::vector<uint8_t>& token, const std::vector<uint8_t>& secret) {
	if (!checkToken(token)) {
		m_server->getLogger().log("Unable to verify " + m_tempPlayerName + "'s encryption tokens", "Network", Logger::WARN);
		return false;
	}

	if (secret.size() != 16) {
		m_server->getLogger().log("Unable to verify " + m_tempPlayerName + "'s encryption secret", "Network", Logger::WARN);
		return false;
	}

	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_secret = secret;
	}

	return true;
}

std::vector<uint8_t> Client::getSecret() {
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_secret;
}

void Client::enableCompression() {
	std::lock_guard<std::mutex> lock(m_mutex);
	m_compressionThreshold.store(256);
}
