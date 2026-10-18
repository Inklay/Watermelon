#include "Utils/Buffer.h"
#include <iostream>
#include <iomanip>
#include <cstdint>
#include "Java/Packet/Packet.h"

namespace Utils {
	namespace Buffer {
		void printBuffer(std::vector<uint8_t>& buffer) {
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

		void printWithSize(std::vector<uint8_t>& buffer) {
			std::vector<uint8_t> fullPacket;

			Java::Packet::writeVarInt((int32_t)buffer.size(), fullPacket);
			fullPacket.insert(fullPacket.end(), buffer.begin(), buffer.end());
			printBuffer(fullPacket);
		}
	}
}
