#pragma once

#include "Core/Client.h"

namespace Auth {
	enum Status {
		AUTHENTIFIED,
		FAILED,
		SERVER_DOWN
	};

	struct Result {
		Status status;
		std::string username;
		std::string uuid;
	};

	Result authenticatePlayer(std::shared_ptr<Client> client, Watermelon* server);
}
