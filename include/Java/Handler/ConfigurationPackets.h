#pragma once

#include "Client.h"
#include "Watermelon.h"

namespace Java {
	namespace Handler {
		namespace Configuration {
			std::vector<char> registryDataPacket(std::shared_ptr<Client> client, Watermelon* server);
		};
	};
};
