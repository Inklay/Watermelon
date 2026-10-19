#include "Compression/ZLIB.h"
#include <zlib.h>

namespace Compression {
	namespace ZLIB {
		std::vector<uint8_t> compress(const std::vector<uint8_t>& data) {
			uLong outputSize = compressBound(static_cast<uLong>(data.size()));
			std::vector<uint8_t> compressed(static_cast<size_t>(outputSize));
			compressed.reserve(outputSize);

			compress2(compressed.data(), &outputSize, data.data(), static_cast<uLong>(data.size()), Z_DEFAULT_COMPRESSION);

			compressed.resize(outputSize);
			return compressed;
		}

		std::vector<uint8_t> decompress(const std::vector<uint8_t>& data, size_t expectedSize) {
			std::vector<uint8_t> decompressed(static_cast<size_t>(expectedSize));
			uLongf outputSize = static_cast<uLongf>(expectedSize);

			uncompress(decompressed.data(), &outputSize, data.data(), static_cast<uLong>(data.size()));

			decompressed.resize(outputSize);
			return decompressed;
		}
	}
}
