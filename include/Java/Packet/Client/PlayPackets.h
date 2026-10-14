#pragma once

#include "Client.h"
#include "Watermelon.h"

namespace Java {
	namespace Packet {
		namespace Client {
			namespace Play {
				void synchronizePlayerPosition(std::shared_ptr<::Client> client, Watermelon* server);
				void login(std::shared_ptr<::Client> client, Watermelon* server);
			};
		};
	};
};
