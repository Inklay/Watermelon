#pragma once

#include "../../Client.h"
#include "../Packet.h"
#include "../../Watermelon.h"

namespace Java {
	namespace Parser {
		namespace LoginPackets {
			void parse(std::shared_ptr<Client> client, const Java::Packet::Header& header, Watermelon* server);
		};
	};
};
