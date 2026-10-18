#pragma once

#include "Core/Client.h"
#include "Core/Watermelon.h"

namespace Java {
	namespace Packet {
		namespace Client {
			namespace Login {
				void loginSuccess(std::shared_ptr<::Client> client, Watermelon* server, const std::string& username, std::pair<uint64_t, uint64_t> uuid);
				void disconnect(std::shared_ptr<::Client> client, Watermelon* server, const std::string& reason);
				void encryptionRequest(std::shared_ptr<::Client> client, Watermelon* server);
			};
		};
	};
};
