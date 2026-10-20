#include "Utils/Base64.h"
#include <openssl/evp.h>

namespace Utils {
	namespace Base64 {

		std::vector<uint8_t> decode(const std::string& data) {
			std::vector<uint8_t> out(data.size() * 3 / 4 + 4);

			int n = EVP_DecodeBlock(out.data(), reinterpret_cast<const unsigned char*>(data.data()), static_cast<int>(data.size()));
			
			if (n < 0) {
				return {};
			}

			size_t padding = 0;
			if (!data.empty() && data.back() == '=') {
				++padding;
			}

			if (data.size() >= 2 && data[data.size() - 2] == '=') {
				++padding;
			}

			out.resize(static_cast<size_t>(n) - padding);
			return out;
		}

		std::string encode(const std::vector<uint8_t>& data) {
			static constexpr char alphabet[] =
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"abcdefghijklmnopqrstuvwxyz"
				"0123456789+/";

			std::string encoded;
			encoded.reserve(((data.size() + 2) / 3) * 4);

			for (size_t i = 0; i < data.size(); i += 3) {
				const uint32_t a = data[i];
				const uint32_t b = i + 1 < data.size() ? data[i + 1] : 0;
				const uint32_t c = i + 2 < data.size() ? data[i + 2] : 0;

				const uint32_t value = (a << 16) | (b << 8) | c;

				encoded += alphabet[(value >> 18) & 0x3F];
				encoded += alphabet[(value >> 12) & 0x3F];
				encoded += i + 1 < data.size() ? alphabet[(value >> 6) & 0x3F] : '=';
				encoded += i + 2 < data.size() ? alphabet[value & 0x3F] : '=';
			}

			return encoded;
		}
	}
}
