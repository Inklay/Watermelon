#pragma once

#include <vector>
#include <cstdint>
#include "Crypto/RSA.h"

class Keys {
public:
	Keys();
	std::vector<uint8_t> getPublicKey();
	std::vector<uint8_t> decryptData(const std::vector<uint8_t>& data);

private:
	const Crypto::RSA::Key m_key;
};
