#pragma once

#include "../../Client.h"
#include "../../Watermelon.h"
#include "../Packet.h"

namespace Java {
	namespace Parser {
		void parsePacket(std::shared_ptr<Client> client, const Java::Packet::Header& header, Watermelon* server);
		Java::Packet::Header parseHeader(std::shared_ptr<Client> client);
	};
}
