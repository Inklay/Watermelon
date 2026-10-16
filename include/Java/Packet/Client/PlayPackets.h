#pragma once

#include "Client.h"
#include "Watermelon.h"
#include "Java/Packet/Packet.h"

namespace Java {
	namespace Packet {
		namespace Client {
			namespace Play {
				void synchronizePlayerPosition(std::shared_ptr<::Client> client, Watermelon* server);
				void login(std::shared_ptr<::Client> client, Watermelon* server);
				void gameEvent(std::shared_ptr<::Client> client, Watermelon* server, GameEvent event, float data = 0.0f);
				void playerInfoUpdate(std::shared_ptr<::Client> client, Watermelon* server, const std::vector<Player::InfoAction>& actions, const std::vector<Player*> players);
				void setCenterChunk(std::shared_ptr<::Client> client, Watermelon* server);
				void chunkDataAndUpdateLight(std::shared_ptr<::Client> client, Watermelon* server, int x, int z);
			};
		};
	};
};
