#include "Client.h"
#include "Watermelon.h"

Client::Client(Type type, Socket socket) :
	m_type(type),
	m_status(Status::HANDSHAKING),
	m_socket(socket),
	m_player(nullptr)
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

void Client::createPlayer(const std::string& username, std::pair<uint64_t, uint64_t> uuid, int32_t entityId, Entity::Position position, Entity::Rotation rotation) {
	std::lock_guard<std::mutex> lock(m_mutex);
	m_player = new Player(username, uuid, entityId, position, rotation);
}

Player* Client::getPlayer() {
	return m_player;
}

void Client::setPacks(const std::vector<Java::Pack> packs) {
	std::lock_guard<std::mutex> lock(m_mutex);
	m_packs = packs;
}

std::vector<Java::Pack>& Client::getPacks() {
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_packs;
}

void Client::setSessionUUID(std::pair<uint64_t, uint64_t> sessionUUID) {
	std::lock_guard<std::mutex> lock(m_mutex);
	m_sessionUUID = sessionUUID;
}
