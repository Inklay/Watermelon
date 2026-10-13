#include "Java/Packet.h"

namespace Java {
    namespace Packet {
        int32_t readVarInt(std::shared_ptr<Client> client, uint32_t& offset) {
            char currentByte;
            int32_t value = 0;
            int shift = 0;

            while (true) {
                int bytes = recv(client->socket, &currentByte, 1, 0);
                if (bytes <= 0) {
                    return -1;
                }

                uint8_t uByte = static_cast<uint8_t>(currentByte);
                value |= static_cast<uint32_t>(uByte & 0x7F) << shift;

                if ((uByte & 0x80) == 0) {
                    break;
                }

                shift += 7;
                if (shift >= 35) {
                    return -1;
                }
            }

            return value;
        }

        std::string readString(std::shared_ptr<Client> client, uint32_t& offset) {
            int32_t length = readVarInt(client, offset);

            if (length == -1) {
                return "";
            }

            std::string result(length, '\0');
            int bytesRead = recv(client->socket, &result[0], length, 0);

            if (bytesRead <= 0) {
                return "";
            }

            return result;
        }

        uint16_t readU16(std::shared_ptr<Client> client, uint32_t& offset) {
            uint16_t value = 0;
            int bytesRead = recv(client->socket, reinterpret_cast<char*>(&value), sizeof(value), 0);

            if (bytesRead <= 0) {
                return 0;
            }

            return ntohs(value);
        }

        int64_t read64(std::shared_ptr<Client> client, uint32_t& offset) {
            int64_t value = 0;
            int bytesRead = recv(client->socket, reinterpret_cast<char*>(&value), sizeof(value), 0);

            if (bytesRead <= 0) {
                return 0;
            }

            return ntohll(value);
        }

        void writeVarInt(std::shared_ptr<Client> client, int32_t value, std::vector<char>& buffer) {
			do {
				uint8_t temp = value & 0x7F;
				value >>= 7;
				if (value != 0) {
					temp |= 0x80;
				}
				buffer.push_back(static_cast<char>(temp));
			} while (value != 0);
        }

        void write64(std::shared_ptr<Client> client, int64_t value, std::vector<char>& buffer) {
            buffer.insert(buffer.end(), reinterpret_cast<const char*>(&value), reinterpret_cast<const char*>(&value) + sizeof(value));
        }

        void writeString(std::shared_ptr<Client> client, const std::string& str, std::vector<char>& buffer) {
			writeVarInt(client, (int)str.size(), buffer);
			for (char c : str) {
				buffer.push_back(c);
			}
        }
    };
};
