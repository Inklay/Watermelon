#pragma once

#include "Client.h"
#include "Watermelon.h"
#include "Java/Packet/Packet.h"

namespace Java {
	namespace Packet {
		namespace Server {
			namespace HandshakePackets {
				void parse(std::shared_ptr<::Client> client, const Java::Packet::Header& header, Watermelon* server);
			};
		};
	};
};
