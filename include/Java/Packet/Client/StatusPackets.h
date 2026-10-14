#pragma once

#include "Client.h"
#include "Watermelon.h"

namespace Java {
	namespace Packet {
		namespace Client {
			namespace Status {
				void statusPacket(std::shared_ptr<::Client> client, Watermelon* server);
			};
		};
	};
};
