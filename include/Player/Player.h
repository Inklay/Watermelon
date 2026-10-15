#pragma once

#include "Entity/Entity.h"
#include <string>

class Player : public Entity {
public:
	Player(const std::string& username, std::pair<uint64_t, uint64_t> uuid, int32_t entityId, Entity::Position position, Entity::Rotation rotation);
	const std::string& getUsername();
	int32_t teleport(Entity::Position position, Entity::Rotation rotation);
	bool finishTeleporting(int32_t teleportId);
	static bool isUsernameValid(const std::string& username);

private:
	std::string m_username;
	std::pair<uint64_t, uint64_t> m_uuid;
	int32_t m_teleportIdCounter;
	bool m_teleporting;
};
