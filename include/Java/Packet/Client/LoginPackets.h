#pragma once

#include "Core/Client.h"
#include "Core/Watermelon.h"

namespace Java {
	namespace Packet {
		namespace Client {
			namespace Login {
				void loginSuccess(const std::shared_ptr<::Client>& client);
				void disconnect(const std::shared_ptr<::Client>& client, const std::string& reason);
				void encryptionRequest(const std::shared_ptr<::Client>& client, Watermelon* server);
				void setCompression(const std::shared_ptr<::Client>& client);
				void loginPluginRequest(const std::shared_ptr<::Client>& client, const std::string& identifier);
				void cookieRequest(const std::shared_ptr<::Client>& client, const std::string& key);
			};
		};
	};
};
