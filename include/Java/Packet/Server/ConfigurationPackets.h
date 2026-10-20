#pragma once

#include "Core/Client.h"
#include "Core/Watermelon.h"
#include "Java/Packet/Packet.h"

namespace Java {
	namespace Packet {
		namespace Server {
			namespace Configuration {
				void parse(const std::shared_ptr<::Client>& client, int32_t packetId, Watermelon* server, const std::vector<uint8_t>& packet);
			};
		};
	};
};
