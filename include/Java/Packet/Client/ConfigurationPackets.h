#pragma once

#include "Core/Client.h"
#include "Core/Watermelon.h"

namespace Java {
	namespace Packet {
		namespace Client {
			namespace Configuration {
				void knownPacks(std::shared_ptr<::Client> client, Watermelon* server);
				void registryData(std::shared_ptr<::Client> client);
				void finishConfiguration(std::shared_ptr<::Client> client);
				void updateTags(std::shared_ptr<::Client> client);
				void pluginMessage(std::shared_ptr<::Client> client, const std::string& channel, const std::string& data);
			};
		};
	};
};
