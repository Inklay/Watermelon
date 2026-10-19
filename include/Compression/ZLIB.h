#pragma once

#include <vector>
#include <cstdint>

namespace Compression {
	namespace ZLIB {
        std::vector<uint8_t> compress(const std::vector<uint8_t>& data);
        std::vector<uint8_t> decompress(const std::vector<uint8_t>& data, size_t expectedSize);
	}
}
