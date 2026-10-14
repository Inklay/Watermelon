#include "Utils/buffer.h"
#include <iostream>
#include <iomanip>

namespace Utils {
	namespace Buffer {
		void printBuffer(std::vector<char>& buffer) {
			for (const auto& c : buffer) {
				std::cout << std::hex
					<< std::setw(2)
					<< std::setfill('0')
					<< static_cast<unsigned int>(
						static_cast<uint8_t>(c))
					<< ' ';
			}

			std::cout << std::dec << std::endl;
		}
	}
}
