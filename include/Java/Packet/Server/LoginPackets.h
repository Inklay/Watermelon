#pragma once

#include "Client.h"
#include "Watermelon.h"
#include "Java/Packet/Packet.h"

namespace Java {
	namespace Packet {
		namespace Server {
			namespace Login {
				void parse(std::shared_ptr<::Client> client, const Java::Packet::Header& header, Watermelon* server);
			};
		};
	};
};
