#include "Player/Player.h"

Player::Player(const std::string& username, std::pair<uint64_t, uint64_t> uuid, int32_t entityId, Entity::Position position, Entity::Rotation rotation)
	: Entity(entityId, position, rotation, Entity::Type::PLAYER),
	m_username(username),
	m_uuid(uuid),
	m_teleportIdCounter(0),
	m_teleporting(false)
{
}

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
