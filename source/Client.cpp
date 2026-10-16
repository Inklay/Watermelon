#include "Client.h"
#include "Watermelon.h"
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

bool Client::createPlayer(const std::string& username, std::pair<uint64_t, uint64_t> uuid, int32_t entityId, Entity::Position position, Entity::Rotation rotation) {
	bool disconnect = false;
	std::string logMessage;
	
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_player != nullptr) {
			disconnect = true;
			logMessage = "Player was already logged in";
		} else {
			if (!Player::isUsernameValid(username)) {
				disconnect = true;
				logMessage = "Invalid player name";
			} else {
				m_player = new Player(username, uuid, entityId, position, rotation);
			}
		}
	}

	if (disconnect) {
		m_server->getLogger().log(logMessage, "Network", Logger::WARN);
		m_server->disconnectClient(this);
		return false;
	}

	return true;
}

Player* Client::getPlayer() {
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_player;
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

	packet.assign(
		m_receiveBuffer.begin() + prefixLength,
		m_receiveBuffer.begin() + prefixLength + length
	);

	m_receiveBuffer.erase(
		m_receiveBuffer.begin(),
		m_receiveBuffer.begin() + prefixLength + length
	);

	return Client::PacketStatus::COMPLETE;
}

void Client::appendReceivedData(const uint8_t* data, size_t size) {
	constexpr int32_t MAX_BUFFER_SIZE = 4000000;
	bool disconnect = false;

	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_receiveBuffer.size() + size > MAX_BUFFER_SIZE) {
			disconnect = true;
		} else {
			m_receiveBuffer.insert(m_receiveBuffer.end(), data, data + size);
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

void Client::sendData(const std::vector<char>& data) {
	bool disconnect = false;

	{
		std::lock_guard<std::mutex> lock(m_sendingMutex);
		size_t totalSent = 0;
		size_t size = data.size();

		while (totalSent < size) {
			const size_t remaining = size - totalSent;
			const int toSend = static_cast<int>((std::min)(remaining, static_cast<size_t>(INT_MAX)));
			const int sent = send(m_socket, data.data() + totalSent, toSend, 0);

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
