#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace Utils {
	namespace Base64 {
		std::string encode(const std::vector<uint8_t>& data);
	}
}
