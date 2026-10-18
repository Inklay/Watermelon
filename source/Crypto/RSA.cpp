#include "Crypto/RSA.h"
#include <stdexcept>
#include <openssl/rsa.h>
#include <openssl/x509.h>

namespace Crypto {
	namespace RSA {
		Key generateKeyPair(int bits) {
			if (bits < 1024) {
				throw std::invalid_argument("RSA key too small");
			}

			EVP_PKEY_CTX* rawCtx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);

			if (!rawCtx) {
				throw std::runtime_error("EVP_PKEY_CTX_new_id failed");
			}

			std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)> ctx(rawCtx, EVP_PKEY_CTX_free);

			if (EVP_PKEY_keygen_init(ctx.get()) < 0) {
				throw std::runtime_error("RSA keygen initialization failed");
			}

			if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx.get(), bits) < 0) {
				throw std::runtime_error("RSA key size configuration failed");
			}

			EVP_PKEY* key = nullptr;

			if (EVP_PKEY_keygen(ctx.get(), &key) < 0) {
				throw std::runtime_error("RSA key generation failed");
			}

			return Key(key, EVP_PKEY_free);
		}

		std::vector<uint8_t> publicKeyDER(const Key& key) {
			if (!key) {
				throw std::invalid_argument("RSA key is null");
			}

			const int length = i2d_PUBKEY(key.get(), nullptr);

			if (length < 0) {
				throw std::runtime_error("Public key DER encoding failed");
			}

			std::vector<uint8_t> publicKey(static_cast<size_t>(length));
			unsigned char* ptr = publicKey.data();

			if (i2d_PUBKEY(key.get(), &ptr) != length) {
				throw std::runtime_error("Public key DER encoding failed");
			}

			return publicKey;
		}

		std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data, const Key& key) {
			struct CtxDeleter { void operator()(EVP_PKEY_CTX* ctx) const { EVP_PKEY_CTX_free(ctx); } };
			std::unique_ptr<EVP_PKEY_CTX, CtxDeleter> ctx(EVP_PKEY_CTX_new(key.get(), nullptr));

			if (!ctx || EVP_PKEY_decrypt_init(ctx.get()) <= 0 || EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_PADDING) <= 0) {
				throw std::runtime_error("Can't initialize the RSA decryption");
			}

			size_t outLength = 0;

			if (EVP_PKEY_decrypt(ctx.get(), nullptr, &outLength, data.data(), data.size()) <= 0) {
				throw std::runtime_error("RSA decryption failed");
			}

			std::vector<uint8_t> out(outLength);

			if (EVP_PKEY_decrypt(ctx.get(), out.data(), &outLength, data.data(), data.size()) <= 0) {
				throw std::runtime_error("RSA decryption failed");
			}

			out.resize(outLength);
			return out;
		}
	}
}
