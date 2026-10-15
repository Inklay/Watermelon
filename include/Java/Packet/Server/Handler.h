#pragma once

#include "Client.h"
#include "Watermelon.h"
#include "Java/Packet/Packet.h"

namespace Java {
	namespace Packet {
		namespace Server {
			namespace Handler {
				void handlePacket(std::shared_ptr<::Client> client, Watermelon* server, std::vector<uint8_t>& packet);
			};
		};
	};
}
