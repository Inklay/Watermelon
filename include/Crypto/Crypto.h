#pragma once

#include <vector>
#include <cstdint>
#include <string>

namespace Crypto {
	std::vector<uint8_t> generateRandomBytes(int size);
	std::string digestToMinecraftHex(std::vector<uint8_t> digest);
	std::string minecraftDigest(const std::string& serverId, const std::vector<uint8_t>& sharedSecret, const std::vector<uint8_t>& publicKeyDer);
}
