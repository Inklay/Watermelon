#include "Player/ChatSession.h"
#include "Core/Watermelon.h"
#include "Java/Packet/Packet.h"

namespace Player {
	bool ChatSession::validate(Watermelon* server, std::pair<uint64_t, uint64_t> playerUUID) {
		std::vector<uint8_t> signedData;
		signedData.reserve(16 + 8 + publicKey.size());

		Java::Packet::writeUUID(playerUUID, signedData);
		Java::Packet::write64(expiresAt, signedData);
		signedData.insert(signedData.end(), publicKey.begin(), publicKey.end());

		return server->getKeys().checkChatSession(signedData, keySignature);
	}
}
