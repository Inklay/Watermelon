#pragma once

#include "Entity/Entity.h"
#include <string>
#include <vector>
#include "Player/ProfileProperty.h"
#include "Player/ChatSession.h"
#include <atomic>

class Watermelon;
class Client;

namespace Player {
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

	enum ChatMode {
		ENABLED = 0,
		COMMANDS_ONLY = 1,
		OFF = 2
	};

	struct ClientInfo {
		uint8_t viewDistance = 10;
		ChatMode chatMode = ChatMode::ENABLED;
		uint8_t skinPart = 1;
		uint32_t mainHand = 1;
		bool shownOnServerList = false;
		uint32_t particles = 0;
	};

	bool isUsernameValid(const std::string& username);

	class Player : public Entity {
	public:
		Player(const std::string& username, std::pair<uint64_t, uint64_t> uuid, int32_t entityId, Entity::Position position, Entity::Rotation rotation, Watermelon* server, Client* client);
		const std::string& getUsername();
		int32_t teleport(Entity::Position position, Entity::Rotation rotation);
		bool finishTeleporting(int32_t teleportId);
		std::pair<uint64_t, uint64_t> getUUID();
		Gamemode getGamemode();
		void setGamemode(Gamemode gamemode);
		Position getPosition();
		bool hasChatSignature();
		bool isTeleporting();
		void setClientInfo(ClientInfo clientInfo);
		const ClientInfo& getClientInfo();
		void setProfileProperties(std::vector<ProfileProperty> profileProperties);
		const std::vector<ProfileProperty>& getProfileProperties();
		void setChatSession(ChatSession chatSession);
		void setLoading(bool loading);
		bool isLoaded();
		void clientTickEnd();
		void tickEnd();
		bool setPosition(Position position);
		void setRotation(Rotation rotation);

	private:
		bool isNewPositionValid(Position& position);

		std::string m_username;
		std::pair<uint64_t, uint64_t> m_uuid;
		int32_t m_teleportIdCounter;
		bool m_teleporting;
		Gamemode m_gamemode;
		ClientInfo m_clientInfo;
		std::vector<ProfileProperty> m_profileProperties;
		ChatSession m_chatSession;
		Watermelon* m_server;
		std::atomic<bool> m_loading;
		int m_loadingTickCounter;
		int m_movementPacketCounter;
		int m_knownMovementPacketCounter;
		Client* m_client;
		Position m_lastGoodPosition;
	};
}
