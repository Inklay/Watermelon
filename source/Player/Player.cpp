#include "Player/Player.h"
#include "Core/Watermelon.h"
#include <algorithm>
#include "Core/Client.h"
#include "Java/Packet/Client/PlayPackets.h"
#include <nlohmann/json.hpp>

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

	Player::Player(const std::string& username, std::pair<uint64_t, uint64_t> uuid, int32_t entityId, Entity::Position position, Entity::Rotation rotation, Watermelon* server, Client* client)
		: Entity(entityId, position, rotation, Entity::Type::PLAYER),
		m_username(username),
		m_uuid(uuid),
		m_teleportIdCounter(0),
		m_teleporting(false),
		m_gamemode(CREATIVE), // FIXME must be survival once it is implemented
		m_server(server),
		m_loading({ true }),
		m_loadingTickCounter(0),
		m_movementPacketCounter(0),
		m_knownMovementPacketCounter(0),
		m_client(client),
		m_lastGoodPosition(position),
		m_inputFlag(0),
		m_isFlying(false)
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
		m_profileProperties = std::move(profileProperties);
	}

	const std::vector<ProfileProperty>& Player::getProfileProperties() {
		return m_profileProperties;
	}

	void Player::setChatSession(ChatSession chatSession) {
		if (!chatSession.validate(m_server, m_uuid)) {
			if (m_server->getKeys().getMojangPublicKeys().size() != 0) {
				m_server->getLogger().log("Can't validate " + m_username + "'s chat session", "Network", Logger::WARN);
			}
		} else {
			m_chatSession = std::move(chatSession);
		}
	}

	void Player::setLoading(bool loading) {
		m_loading.store(loading);
	}

	bool Player::isLoaded() {
		return !m_loading.load();
	}

	void Player::clientTickEnd() {
		if (++m_loadingTickCounter >= 60) {
			m_loading.store(false);
		}
	}

	void Player::tickEnd() {
		m_knownMovementPacketCounter = m_movementPacketCounter;
		m_lastGoodPosition = m_position;
	}

	bool Player::isNewPositionValid(Position& position) {
		if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z)) {
			Java::Packet::Client::Play::disconnect(m_client->shared_from_this(), m_server, nlohmann::json({{"translate", "multiplayer.disconnect.invalid_player_movement"}}).dump());
			return false;
		}

		constexpr double MAX_HORIZONTAL = 30'000'000.0;
		constexpr double MAX_VERTICAL = 20'000'000.0;

		position.x = std::clamp(position.x, -MAX_HORIZONTAL, MAX_HORIZONTAL);
		position.y = std::clamp(position.y, -MAX_VERTICAL, MAX_VERTICAL);
		position.z = std::clamp(position.z, -MAX_HORIZONTAL, MAX_HORIZONTAL);

		return true;
	}

	void Player::setRotation(Rotation rotation) {
		if (m_teleporting) {
			return;
		}

		m_rotation = std::move(rotation);
	}

	bool Player::setPosition(Position position) {
		m_movementPacketCounter++;

		if (m_teleporting) {
			return false;
		}

		if (!isNewPositionValid(position)) {
			return false;
		}

		uint32_t packetBudget = m_movementPacketCounter - m_knownMovementPacketCounter;

		if (packetBudget > 5) {
			packetBudget = 1;
		}

		const double dx = position.x - m_lastGoodPosition.x;
		const double dy = position.y - m_lastGoodPosition.y;
		const double dz = position.z - m_lastGoodPosition.z;

		const double distanceSquared = dx * dx + dy * dy + dz * dz;
		const double velocitySquared = 0.0; // FIXME player velocity

		if (distanceSquared - velocitySquared > 100.0 * static_cast<double>(packetBudget)) {
			m_server->getLogger().log(m_username + " moved to quickly", "Watermelon", Logger::INFO);
			Java::Packet::Client::Play::synchronizePlayerPosition(m_client->shared_from_this());
			return false;
		}

		m_position = std::move(position);
		return true;
	}

	void Player::setInputFlag(uint8_t inputFlag) {
		m_inputFlag = inputFlag;
	}

	void Player::setFlying(bool isFlying) {
		m_isFlying = isFlying;
	}

	bool Player::isFlying() {
		return m_isFlying;
	}
}
