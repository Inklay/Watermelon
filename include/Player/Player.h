#pragma once

#include "Entity/Entity.h"
#include <string>
#include <vector>
#include "Player/ProfileProperty.h"
#include "Player/ChatSession.h"
#include <atomic>
#include <unordered_set>

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
		Player(const std::string& username, std::pair<uint64_t, uint64_t> uuid, uint32_t entityId, Entity::Vec3 position, Entity::Rotation rotation, Watermelon* server, Client* client);
		const std::string& getUsername() const;
		int32_t teleport(Entity::Vec3 position, Entity::Rotation rotation);
		bool finishTeleporting(int32_t teleportId);
		Gamemode getGamemode() const;
		void setGamemode(Gamemode gamemode);
		bool hasChatSignature() const;
		bool isTeleporting() const;
		void setClientInfo(ClientInfo clientInfo);
		const ClientInfo& getClientInfo() const;
		void setProfileProperties(std::vector<ProfileProperty> profileProperties);
		const std::vector<ProfileProperty>& getProfileProperties() const;
		void setChatSession(ChatSession chatSession);
		void setLoading(bool loading);
		bool isLoaded() const;
		void clientTickEnd();
		void tickEnd();
		void tick();
		bool setPosition(Vec3 position);
		void setRotation(Rotation rotation);
		void setInputFlag(uint8_t inputFlag);
		void setFlying(bool isFlying);
		bool isFlying() const;
		void syncEntities();
		uint8_t getEntityRange() const;
		std::shared_ptr<Client> getClient() const;

	private:
		bool isNewPositionValid(Vec3& position);

		std::string m_username;
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
		uint8_t m_inputFlag;
		bool m_isFlying;
		std::unordered_set<int32_t> m_trackedEntities;
		std::unordered_set<int32_t> m_visibleEntitiesNow;
		std::vector<uint32_t> m_entitiesVec;
		std::unordered_set<uint32_t> m_knownPlayers;
	};
}
