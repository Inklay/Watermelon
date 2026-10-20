#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <memory>
#include <openssl/evp.h>

namespace Crypto {
	using Key = std::shared_ptr<EVP_PKEY>;
	std::vector<uint8_t> generateRandomBytes(int size);
	std::string digestToMinecraftHex(std::vector<uint8_t> digest);
	std::string minecraftDigest(const std::string& serverId, const std::vector<uint8_t>& sharedSecret, const std::vector<uint8_t>& publicKeyDer);
	Key DER2EVP(const std::vector<uint8_t>& der);
}
