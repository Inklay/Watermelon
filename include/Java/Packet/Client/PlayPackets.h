#pragma once

#include "Core/Client.h"
#include "Core/Watermelon.h"
#include "Java/Packet/Packet.h"

namespace Java {
	namespace Packet {
		namespace Client {
			namespace Play {
				void synchronizePlayerPosition(const std::shared_ptr<::Client>& client);
				void login(const std::shared_ptr<::Client>& client, Watermelon* server);
				void gameEvent(const std::shared_ptr<::Client>& client, GameEvent event, float data = 0.0f);
				void playerInfoUpdateAll(Watermelon* server, const std::vector<Player::InfoAction>& actions, const std::vector<const Player::Player*>& players, const std::shared_ptr<::Client>& sendingClient);
				void playerInfoUpdateOne(const std::vector<Player::InfoAction>& actions, const std::vector<const Player::Player*>& players, const std::shared_ptr<::Client>& client);
				void setCenterChunk(const std::shared_ptr<::Client>& client);
				void chunkDataAndUpdateLight(const std::shared_ptr<::Client>& client, int x, int z);
				void disconnect(const std::shared_ptr<::Client>& client, const std::string& reason);
				void keepAlive(const std::shared_ptr<::Client>& client, uint64_t keepAliveid);
				void spawnEntity(const std::shared_ptr<::Client>& client, Entity* entity);
				void removeEntities(const std::shared_ptr<::Client>& client, std::vector<uint32_t> ids);
			};
		};
	};
};
