#include "Crypto/Crypto.h"
#include <openssl/rand.h>
#include <openssl/rand.h>

namespace Crypto {
	std::vector<uint8_t> generateRandomBytes(int size) {
        std::vector<uint8_t> data(size);
        RAND_bytes(data.data(), static_cast<int>(data.size()));

        return data;
	}
}
