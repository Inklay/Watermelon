#include "Player/Player.h"
#include "Core/Watermelon.h"

namespace Player {
	bool isUsernameValid(const std::string& username) {
		if (username.empty() || username.size() > 16) {
			return false;
		}

		for (const char c : username) {
			const bool ok = (c >= 'a' && c <= 'z')
				|| (c >= 'A' && c <= 'Z')
				|| (c >= '0' && c <= '9')
				|| c == '_';

			if (!ok) {
				return false;
			}
		}

		return true;
	}

	Player::Player(const std::string& username, std::pair<uint64_t, uint64_t> uuid, int32_t entityId, Entity::Position position, Entity::Rotation rotation, Watermelon* server)
		: Entity(entityId, position, rotation, Entity::Type::PLAYER),
		m_username(username),
		m_uuid(uuid),
		m_teleportIdCounter(0),
		m_teleporting(false),
		m_gamemode(CREATIVE), // FIXME must be survival once it is implemented
		m_server(server)
	{}

	const std::string& Player::getUsername() {
		return m_username;
	}

	int32_t Player::teleport(Entity::Position position, Entity::Rotation rotation) {
		if (m_teleporting) {
			return -1;
		}

		m_teleporting = true;
		m_position = position;
		m_rotation = rotation;

		return ++m_teleportIdCounter;
	}

	bool Player::finishTeleporting(int32_t teleportId) {
		if (!m_teleporting || teleportId != m_teleportIdCounter) {
			return false;
		}

		m_teleporting = false;
		return true;
	}

	std::pair<uint64_t, uint64_t> Player::getUUID() {
		return m_uuid;
	}

	Gamemode Player::getGamemode() {
		return m_gamemode;
	}

	void Player::setGamemode(Gamemode gamemode) {
		m_gamemode = gamemode;
	}

	Player::Position Player::getPosition() {
		return m_position;
	}

	bool Player::hasChatSignature() {
		return false; // FIXME add real chat signature
	}

	bool Player::isTeleporting() {
		return m_teleporting;
	}

	void Player::setClientInfo(ClientInfo clientInfo) {
		m_clientInfo = clientInfo;
	}

	const ClientInfo& Player::getClientInfo() {
		return m_clientInfo;
	}

	void Player::setProfileProperties(std::vector<ProfileProperty> profileProperties) {
		m_profileProperties = profileProperties;
	}

	const std::vector<ProfileProperty>& Player::getProfileProperties() {
		return m_profileProperties;
	}

	void Player::setChatSession(ChatSession chatSession) {
		if (!chatSession.validate(m_server, m_uuid)) {
			m_server->getLogger().log("Can't validate " + m_username + "'s chat session", "Network", Logger::WARN);
		} else {
			m_chatSession = chatSession;
		}
	}
}
