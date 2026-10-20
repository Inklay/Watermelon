#pragma once

#include <vector>
#include <cstdint>
#include <memory>
#include <openssl/evp.h>

namespace Crypto {
	namespace CFB8 {
		class Cipher {
		public:
			Cipher();
			void init(const std::vector<uint8_t>& secret);
			std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data);
			std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data);

		private:
			std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> m_encryptCtx;
			std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> m_decryptCtx;
		};
	}
}
