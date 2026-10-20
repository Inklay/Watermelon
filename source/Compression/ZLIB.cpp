#include "Compression/ZLIB.h"
#include <zlib.h>
#include <stdexcept>

namespace Compression {
	namespace ZLIB {
		std::vector<uint8_t> compress(const std::vector<uint8_t>& data) {
			uLong outputSize = compressBound(static_cast<uLong>(data.size()));
			std::vector<uint8_t> compressed(static_cast<size_t>(outputSize));
			compressed.reserve(outputSize);

			int result = compress2(compressed.data(), &outputSize, data.data(), static_cast<uLong>(data.size()), Z_DEFAULT_COMPRESSION);

			if (result != Z_OK) {
				throw std::runtime_error("Compression error");
			}

			compressed.resize(outputSize);
			return compressed;
		}

		std::vector<uint8_t> decompress(const std::vector<uint8_t>& data, size_t expectedSize) {
			std::vector<uint8_t> decompressed(static_cast<size_t>(expectedSize));
			uLongf outputSize = static_cast<uLongf>(expectedSize);

			int result = uncompress(decompressed.data(), &outputSize, data.data(), static_cast<uLong>(data.size()));

			if (result != Z_OK || outputSize != expectedSize) {
				throw std::runtime_error("Decompression error");
			}

			decompressed.resize(outputSize);
			return decompressed;
		}
	}
}
