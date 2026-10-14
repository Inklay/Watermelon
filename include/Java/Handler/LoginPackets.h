#pragma once

#include "Client.h"
#include "Watermelon.h"

namespace Java {
	namespace Handler {
		namespace Login {
			std::vector<char> loginStartPacket(std::shared_ptr<Client> client, Watermelon* server, const std::string& username, std::pair<uint64_t, uint64_t> uuid);
		};
	};
};
