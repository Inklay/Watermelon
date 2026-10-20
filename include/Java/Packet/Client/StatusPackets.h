#pragma once

#include "Core/Client.h"
#include "Core/Watermelon.h"

namespace Java {
	namespace Packet {
		namespace Client {
			namespace Status {
				void statusPacket(const std::shared_ptr<::Client>& client, Watermelon* server);
			};
		};
	};
};
