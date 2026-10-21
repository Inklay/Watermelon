#include "Player/ChatSession.h"
#include "Core/Watermelon.h"
#include "Java/Packet/Packet.h"
#include <chrono>

namespace Player {
	bool ChatSession::validate(Watermelon* server, std::pair<uint64_t, uint64_t> playerUUID) {
		uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		if (now > expiresAt) {
			return false;
		}

		std::vector<uint8_t> signedData;
		signedData.reserve(16 + 8 + publicKey.size());

		Java::Packet::writeUUID(playerUUID, signedData);
		Java::Packet::write64(expiresAt, signedData);
		signedData.insert(signedData.end(), publicKey.begin(), publicKey.end());

		return server->getKeys().checkChatSession(signedData, keySignature);
	}
}
