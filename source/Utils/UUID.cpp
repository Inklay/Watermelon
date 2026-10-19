#include "Utils/UUID.h"
#include <iomanip>
#include <sstream>
#include <random>
#include <openssl/evp.h>

namespace Utils {
	namespace UUID {
		std::string formatUUID(const std::pair<uint64_t, uint64_t>& uuid) {
            uint64_t mostSigBits = uuid.first;
            uint64_t leastSigBits = uuid.second;

            std::stringstream ss;
            ss << std::hex << std::setfill('0')
                << std::setw(8) << ((mostSigBits >> 32) & 0xFFFFFFFF) << "-"
                << std::setw(4) << ((mostSigBits >> 16) & 0xFFFF) << "-"
                << std::setw(4) << (mostSigBits & 0xFFFF) << "-"
                << std::setw(4) << ((leastSigBits >> 48) & 0xFFFF) << "-"
                << std::setw(12) << (leastSigBits & 0xFFFFFFFFFFFF);

            return ss.str();
		}

		std::pair<uint64_t, uint64_t> generateSessionUUID() {
            std::random_device rd;
            std::mt19937_64 generator(rd());
            std::uniform_int_distribution<uint64_t> distribution;
            uint64_t msb = distribution(generator);
            uint64_t lsb = distribution(generator);

            msb = (msb & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
            lsb = (lsb & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

            return { msb, lsb };
		}

		std::pair<uint64_t, uint64_t> offlineUUID(const std::string& username) {
			const std::string input = "OfflinePlayer:" + username;
			uint8_t digest[EVP_MAX_MD_SIZE];
			unsigned int length = 0;

			if (EVP_Digest(input.data(), input.size(), digest, &length, EVP_md5(), nullptr) != 1 || length != 16) {
				throw std::runtime_error("MD5 failed");
			}

			digest[6] = (digest[6] & 0x0F) | 0x30;
			digest[8] = (digest[8] & 0x3F) | 0x80;

			uint64_t msb = 0, lsb = 0;
			for (int i = 0; i < 8; i++) {
				msb = (msb << 8) | digest[i];
				lsb = (lsb << 8) | digest[i + 8];
			}

			return { msb, lsb };
		}
	};
};
