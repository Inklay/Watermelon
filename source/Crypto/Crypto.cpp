#include "Crypto/Crypto.h"
#include <openssl/rand.h>
#include <stdexcept>
#include <openssl/x509.h>

namespace Crypto {
	std::vector<uint8_t> generateRandomBytes(int size) {
        std::vector<uint8_t> data(size);
        RAND_bytes(data.data(), static_cast<int>(data.size()));

        return data;
	}

	std::string digestToMinecraftHex(std::vector<uint8_t> digest) {
		const bool negative = !digest.empty() && (digest[0] & 0x80);

		if (negative) {
			for (uint8_t& b : digest) {
				b = static_cast<uint8_t>(~b);
			}

			for (size_t i = digest.size(); i-- > 0;) {
				if (++digest[i] != 0) {
					break;
				}
			}
		}

		static constexpr char HEX[] = "0123456789abcdef";
		std::string hex;

		for (const uint8_t b : digest) {
			hex += HEX[b >> 4];
			hex += HEX[b & 0x0F];
		}

		const size_t first = hex.find_first_not_of('0');
		hex = first == std::string::npos ? "0" : hex.substr(first);
		return negative ? "-" + hex : hex;
	}

	std::string minecraftDigest(const std::string& serverId, const std::vector<uint8_t>& sharedSecret, const std::vector<uint8_t>& publicKeyDer) {
		std::vector<uint8_t> input(serverId.begin(), serverId.end());
		input.insert(input.end(), sharedSecret.begin(), sharedSecret.end());
		input.insert(input.end(), publicKeyDer.begin(), publicKeyDer.end());

		std::vector<uint8_t> digest(EVP_MAX_MD_SIZE);
		unsigned int digestLength = 0;

		if (EVP_Digest(input.data(), input.size(), digest.data(), &digestLength, EVP_sha1(), nullptr) != 1) {
			throw std::runtime_error("SHA-1 failed");
		}

		digest.resize(digestLength);
		return digestToMinecraftHex(std::move(digest));
	}

	Key DER2EVP(const std::vector<uint8_t>& der) {
		if (der.empty()) {
			return nullptr;
		}

		const unsigned char* ptr = der.data();

		return Key(d2i_PUBKEY(nullptr, &ptr, static_cast<long>(der.size())), EVP_PKEY_free);
	}
}
