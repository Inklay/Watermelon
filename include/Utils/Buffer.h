#pragma once

#include <vector>
#include <cstdint>

namespace Utils {
	namespace Buffer {
		void printBuffer(std::vector<uint8_t>& buffer);
		void printWithSize(std::vector<uint8_t>& buffer);
	}
}
