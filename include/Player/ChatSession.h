#pragma once

#include <string>
#include <vector>
#include <cstdint>

class Watermelon;

namespace Player {
	class ChatSession {
	public:
		bool validate(Watermelon* server, std::pair<uint64_t, uint64_t> playerUUID);

		std::pair<uint64_t, uint64_t> uuid;
		uint64_t expiresAt;
		std::vector<uint8_t> publicKey;
		std::vector<uint8_t> keySignature;
	};
}
