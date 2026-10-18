#pragma once

#include <openssl/rsa.h>
#include <vector>
#include <cstdint>
#include <memory>

namespace Crypto {
	namespace RSA {
		using Key = std::shared_ptr<EVP_PKEY>;

		Key generateKeyPair(int bits = 1024);
		std::vector<uint8_t> publicKeyDER(const Key& key);
		std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data, const Key& key);
	}
}
