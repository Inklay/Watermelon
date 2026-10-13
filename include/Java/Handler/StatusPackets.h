#pragma once

#include "Client.h"
#include "Watermelon.h"

namespace Java {
	namespace Handler {
		namespace Status {
			std::vector<char> statusPacket(std::shared_ptr<Client> client, Watermelon* server);
		};
	};
};
