#include "Player/Player.h"
#include "Core/Watermelon.h"
#include <algorithm>
#include "Core/Client.h"
#include "Java/Packet/Client/PlayPackets.h"
#include <nlohmann/json.hpp>
#include "Entity/EntityManager.h"

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

	Player::Player(const std::string& username, std::pair<uint64_t, uint64_t> uuid, uint32_t entityId, Entity::Vec3 position, Entity::Rotation rotation, Watermelon* server, Client* client)
		: Entity(entityId, position, rotation, Entity::Type::PLAYER, uuid),
		m_username(username),
		m_teleportIdCounter(0),
		m_teleporting(false),
		m_gamemode(CREATIVE), // FIXME must be survival once it is implemented
		m_server(server),
		m_loading({ true }),
		m_loadingTickCounter(0),
		m_movementPacketCounter(0),
		m_knownMovementPacketCounter(0),
		m_client(client),
		m_inputFlag(0),
		m_isFlying(false)
	{
	}

	const std::string& Player::getUsername() const {
		return m_username;
	}

	int32_t Player::teleport(Entity::Vec3 position, Entity::Rotation rotation) {
		if (m_teleporting) {
			return -1;
		}

		m_teleporting = true;
		m_position = std::move(position);
		updateChunk();
		m_rotation = std::move(rotation);

		return ++m_teleportIdCounter;
	}

	bool Player::finishTeleporting(int32_t teleportId) {
		if (!m_teleporting || teleportId != m_teleportIdCounter) {
			return false;
		}

		m_teleporting = false;
		return true;
	}

	Gamemode Player::getGamemode() const {
		return m_gamemode;
	}

	void Player::setGamemode(Gamemode gamemode) {
		m_gamemode = gamemode;
	}

	bool Player::hasChatSignature() const {
		return false; // FIXME add real chat signature
	}

	bool Player::isTeleporting() const {
		return m_teleporting;
	}

	void Player::setClientInfo(ClientInfo clientInfo) {
		m_clientInfo = clientInfo;
	}

	const ClientInfo& Player::getClientInfo() const {
		return m_clientInfo;
	}

	void Player::setProfileProperties(std::vector<ProfileProperty> profileProperties) {
		m_profileProperties = std::move(profileProperties);
	}

	const std::vector<ProfileProperty>& Player::getProfileProperties() const {
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

	bool Player::isLoaded() const {
		return !m_loading.load();
	}

	void Player::clientTickEnd() {
		if (++m_loadingTickCounter >= 60) {
			m_loading.store(false);
		}
	}

	void Player::syncEntities() {
		m_entitiesVec.clear();
		m_visibleEntitiesNow.clear();
		EntityManager::getInstance().getEntityAround(m_chunkPosition, m_entitiesVec, 1); // FIXME 555555

		for (uint32_t entityId : m_entitiesVec) {
			if (entityId == m_entityId) {
				continue;
			}

			const std::shared_ptr<Entity> entity = EntityManager::getInstance().getEntity(entityId);
			if (entity.get() == nullptr) {
				continue;
			}

			if (m_trackedEntities.contains(entityId)) {
				bool changedPosition = entity->changedPosition();
				bool changedRotation = entity->changedRotation();

				if (changedPosition && changedRotation) {
					Java::Packet::Client::Play::updateEntityPositionAndRotation(m_client->shared_from_this(), entity);
				} else if (changedPosition) {
					Java::Packet::Client::Play::updateEntityPosition(m_client->shared_from_this(), entity);
				} else if (changedRotation) {
					Java::Packet::Client::Play::updateEntityRotation(m_client->shared_from_this(), entity);
				}
			} else {
				if (entity->getType() == Type::PLAYER) {
					const std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(entity);
					if (!player->isLoaded()) {
						continue;
					}

					if (!m_knownPlayers.contains(entityId)) {
						std::vector<InfoAction> actions = { InfoAction::ADD_PLAYER, InfoAction::UPDATE_LISTED, InfoAction::UPDATE_GAME_MODE, InfoAction::UPDATE_LATENCY };
						Java::Packet::Client::Play::playerInfoUpdateOne(actions, { player }, m_client->shared_from_this());
						m_knownPlayers.insert(entityId);
					}
				}

				Java::Packet::Client::Play::spawnEntity(m_client->shared_from_this(), entity);
			}
			m_visibleEntitiesNow.insert(entityId);
		}

		std::vector<uint32_t> removed;
		for (uint32_t id : m_trackedEntities) {
			if (!m_visibleEntitiesNow.contains(id)) {
				removed.push_back(id);

				if (m_knownPlayers.contains(id)) {
					m_knownPlayers.erase(id);
				}
			}
		}

		if (!removed.empty()) {
			Java::Packet::Client::Play::removeEntities(m_client->shared_from_this(), removed);
		}

		std::swap(m_trackedEntities, m_visibleEntitiesNow);
	}

	void Player::tick() {
		syncEntities();
	}

	void Player::tickEnd() {
		m_knownMovementPacketCounter = m_movementPacketCounter;
		m_lastGoodPosition = m_position;
		m_changedPosition = false;
		m_changedRotation = false;
	}

	bool Player::isNewPositionValid(Vec3& position) {
		if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z)) {
			m_server->disconnectClient(m_client, std::string("multiplayer.disconnect.invalid_player_movement"));
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

		m_rotation = { rotation.yaw, rotation.pitch, rotation.yaw };
		m_changedRotation = true;
	}

	bool Player::setPosition(Vec3 position) {
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
		m_changedPosition = true;
		updateChunk();
		return true;
	}

	void Player::setInputFlag(uint8_t inputFlag) {
		m_inputFlag = inputFlag;
	}

	void Player::setFlying(bool isFlying) {
		m_isFlying = isFlying;
	}

	bool Player::isFlying() const {
		return m_isFlying;
	}
}
