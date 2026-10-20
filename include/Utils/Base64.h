#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace Utils {
	namespace Base64 {
		std::vector<uint8_t> decode(const std::string& data);
		std::string encode(const std::vector<uint8_t>& data);
	}
}
