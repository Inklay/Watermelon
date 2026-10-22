#pragma once

#include "Core/Client.h"
#include "Core/Watermelon.h"

namespace Java {
	namespace Packet {
		namespace Client {
			namespace Configuration {
				void knownPacks(const std::shared_ptr<::Client>& client, Watermelon* server);
				void registryData(const std::shared_ptr<::Client>& client);
				void finishConfiguration(const std::shared_ptr<::Client>& client);
				void updateTags(const std::shared_ptr<::Client>& client);
				void pluginMessage(const std::shared_ptr<::Client>& client, const std::string& channel, const std::string& data);
				void disconnect(const std::shared_ptr<::Client>& client, Watermelon* server, const std::string& reason);
			};
		};
	};
};
