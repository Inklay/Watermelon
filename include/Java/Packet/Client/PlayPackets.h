#pragma once

#include "Core/Client.h"
#include "Core/Watermelon.h"
#include "Java/Packet/Packet.h"

namespace Java {
	namespace Packet {
		namespace Client {
			namespace Play {
				void synchronizePlayerPosition(std::shared_ptr<::Client> client);
				void login(std::shared_ptr<::Client> client, Watermelon* server);
				void gameEvent(std::shared_ptr<::Client> client, GameEvent event, float data = 0.0f);
				void playerInfoUpdate(Watermelon* server, const std::vector<Player::InfoAction>& actions, const std::vector<Player::Player*> players);
				void setCenterChunk(std::shared_ptr<::Client> client);
				void chunkDataAndUpdateLight(std::shared_ptr<::Client> client, int x, int z);
			};
		};
	};
};
