#pragma once

#include <vector>
#include <cstdint>
#include "Crypto/RSA.h"
#include "Core/Logger.h"

class Keys {
public:
	Keys(Logger& logger, bool keyMojangPublicKeys);
	std::vector<uint8_t> getPublicKey();
	std::vector<uint8_t> decryptData(const std::vector<uint8_t>& data);
	std::vector<Crypto::Key> getMojangPublicKeys();
	bool checkChatSession(const std::vector<uint8_t>& signedData, const std::vector<uint8_t>& signature);

private:
	void fetchMojangPublicKeys(Logger& logger);

	std::vector<Crypto::Key> m_mojangPublicKeys;
	const Crypto::Key m_key;
};
