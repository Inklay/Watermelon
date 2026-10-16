#pragma once

#include "Entity/Entity.h"
#include <string>

class Player : public Entity {
public:
	enum class InfoAction : uint8_t {
		ADD_PLAYER = 0x01,
		INITIALIZE_CHAT = 0x02,
		UPDATE_GAME_MODE = 0x04,
		UPDATE_LISTED = 0x08,
		UPDATE_LATENCY = 0x10,
		UPDATE_DISPLAY_NAME = 0x20,
		UPDATE_LIST_PRIORITY = 0x40,
		UPDATE_HAT = 0x80
	};

	enum Gamemode {
		SURVIVAL = 0,
		CREATIVE = 1,
		ADVENTURE = 2,
		SPECTATOR = 3
	};

	Player(const std::string& username, std::pair<uint64_t, uint64_t> uuid, int32_t entityId, Entity::Position position, Entity::Rotation rotation);
	const std::string& getUsername();
	int32_t teleport(Entity::Position position, Entity::Rotation rotation);
	bool finishTeleporting(int32_t teleportId);
	static bool isUsernameValid(const std::string& username);
	std::pair<uint64_t, uint64_t> getUUID();
	Gamemode getGamemode();
	void setGamemode(Gamemode gamemode);
	Position getPosition();

private:
	std::string m_username;
	std::pair<uint64_t, uint64_t> m_uuid;
	int32_t m_teleportIdCounter;
	bool m_teleporting;
	Gamemode m_gamemode;
};
