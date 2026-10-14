#pragma once

#include "Client.h"
#include "Watermelon.h"

namespace Java {
	namespace Packet {
		namespace Client {
			namespace Configuration {
				void knownPacks(std::shared_ptr<::Client> client, Watermelon* server);
				void registryData(std::shared_ptr<::Client> client, Watermelon* server);
			};
		};
	};
};
