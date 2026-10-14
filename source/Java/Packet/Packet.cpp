#include "Java/Packet/Packet.h"
#include <iostream>
namespace Java {
    namespace Packet {
        std::string readString(std::shared_ptr<::Client> client, uint32_t& offset) {
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

        int32_t readVarInt(std::shared_ptr<::Client> client, uint32_t& offset) {
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

        uint8_t readU8(std::shared_ptr<::Client> client, uint32_t& offset) {
            uint8_t value = 0;
            int bytesRead = recv(client->socket, reinterpret_cast<char*>(&value), sizeof(value), 0);

            if (bytesRead <= 0) {
                return 0;
            }

            return value;
        }

        uint16_t readU16(std::shared_ptr<::Client> client, uint32_t& offset) {
            uint16_t value = 0;
            int bytesRead = recv(client->socket, reinterpret_cast<char*>(&value), sizeof(value), 0);

            if (bytesRead <= 0) {
                return 0;
            }

            return ntohs(value);
        }

        uint32_t readU32(std::shared_ptr<::Client> client, uint32_t& offset) {
            uint32_t value = 0;
            int bytesRead = recv(client->socket, reinterpret_cast<char*>(&value), sizeof(value), 0);

            if (bytesRead <= 0) {
                return 0;
            }

            return ntohl(value);
        }

        int64_t read64(std::shared_ptr<::Client> client, uint32_t& offset) {
            int64_t value = 0;
            int bytesRead = recv(client->socket, reinterpret_cast<char*>(&value), sizeof(value), 0);

            if (bytesRead <= 0) {
                return 0;
            }

            return ntohll(value);
        }

        float readFloat(std::shared_ptr<::Client> client, uint32_t& offset) {
            uint32_t value = 0;
            int bytesRead = recv(client->socket, reinterpret_cast<char*>(&value), sizeof(value), 0);

            if (bytesRead <= 0) {
                return 0;
            }

            return ntohf(value);
        }

        double readDouble(std::shared_ptr<::Client> client, uint32_t& offset) {
            uint64_t value = 0;
            int bytesRead = recv(client->socket, reinterpret_cast<char*>(&value), sizeof(value), 0);

            if (bytesRead <= 0) {
                return 0;
            }

            return ntohd(value);
        }

        std::pair<uint64_t, uint64_t> readUUID(std::shared_ptr<Client> client, uint32_t& offset) {
            uint64_t a, b = 0;
            int bytesRead = recv(client->socket, reinterpret_cast<char*>(&a), sizeof(a), 0);

            if (bytesRead <= 0) {
                return { 0, 0 };
            }

            bytesRead = recv(client->socket, reinterpret_cast<char*>(&b), sizeof(b), 0);

            if (bytesRead <= 0) {
                return { 0, 0 };
            }

            return std::make_pair(ntohll(a), ntohll(b));
        }

        std::vector<Pack> readPacks(std::shared_ptr<Client> client, uint32_t& offset) {
            int32_t size = readVarInt(client, offset);
            std::vector<Pack> packs;
            std::string nameSpace, id, version;
            packs.reserve(size);

            for (int i = 0; i < size; i++) {
                nameSpace = readString(client, offset);
                id = readString(client, offset);
                version = readString(client, offset);

                packs.emplace_back(nameSpace, id, version);
            }

            return packs;
        }

        void writeString(const std::string& str, std::vector<char>& buffer) {
            writeVarInt((int)str.size(), buffer);
            for (char c : str) {
                buffer.push_back(c);
            }
        }

        void writeVarInt(int32_t value, std::vector<char>& buffer) {
            uint32_t unsignedValue = static_cast<uint32_t>(value);

            do {
                uint8_t temp = unsignedValue & 0x7F;
                unsignedValue >>= 7;

                if (unsignedValue != 0) {
                    temp |= 0x80;
                }

                buffer.push_back(static_cast<char>(temp));
            } while (unsignedValue != 0);
        }

        void writeBool(bool value, std::vector<char>& buffer) {
            write8(value ? 0x01 : 0x00, buffer);
        }

        void writeU8(uint8_t value, std::vector<char>& buffer) {
            buffer.insert(buffer.end(), reinterpret_cast<const char*>(&value), reinterpret_cast<const char*>(&value) + sizeof(value));
        }

        void write8(int8_t value, std::vector<char>& buffer) {
            buffer.insert(buffer.end(), reinterpret_cast<const char*>(&value), reinterpret_cast<const char*>(&value) + sizeof(value));
        }

        void write16(int16_t value, std::vector<char>& buffer) {
            int16_t networkValue = htons(value);
            buffer.insert(buffer.end(), reinterpret_cast<const char*>(&networkValue), reinterpret_cast<const char*>(&networkValue) + sizeof(networkValue));
        }

        void write32(int32_t value, std::vector<char>& buffer) {
            int32_t networkValue = htonl(value);
            buffer.insert(buffer.end(), reinterpret_cast<const char*>(&networkValue), reinterpret_cast<const char*>(&networkValue) + sizeof(networkValue));
        }

        void write64(int64_t value, std::vector<char>& buffer) {
            int64_t networkValue = htonll(value);
            buffer.insert(buffer.end(), reinterpret_cast<const char*>(&networkValue), reinterpret_cast<const char*>(&networkValue) + sizeof(networkValue));
        }

        void writeFloat(float value, std::vector<char>& buffer) {
            uint32_t networkValue = htonf(value);
            buffer.insert(buffer.end(), reinterpret_cast<const char*>(&networkValue), reinterpret_cast<const char*>(&networkValue) + sizeof(networkValue));
        }

        void writeDouble(double value, std::vector<char>& buffer) {
            uint64_t networkValue = htond(value);
            buffer.insert(buffer.end(), reinterpret_cast<const char*>(&networkValue), reinterpret_cast<const char*>(&networkValue) + sizeof(networkValue));
        }

        void writeUUID(std::pair<uint64_t, uint64_t> uuid, std::vector<char>& buffer) {
            write64(uuid.first, buffer);
            write64(uuid.second, buffer);
        }

        void writePacks(std::vector<Pack>& packs, std::vector<char>& buffer) {
            writeVarInt((int32_t)packs.size(), buffer);

            for (const Pack& i : packs) {
                writeString(i.nameSpace, buffer);
                writeString(i.id, buffer);
                writeString(i.version, buffer);
            }
        }

        void writeRegistry(const Registry* registry, bool skipNbt, std::vector<char>& buffer) {
            writeString(registry->name, buffer);
            writeVarInt((int32_t)registry->entries.size(), buffer);
            
            for (const Registry::Entry& entry : registry->entries) {
                writeString(entry.textId, buffer);
                if (skipNbt) {
                    writeBool(false, buffer);
                } else {
                    writeBool(true, buffer);
                    writeNBT(entry.nbtPayload, buffer);
                }
            }
        }

        void writeNBT(const std::vector<uint8_t>& data, std::vector<char>& buffer) {
            for (const uint8_t byte : data) {
                writeU8(byte, buffer);
            }
        }

        void writeTags(const std::vector<RegistryTag>& tags, std::vector<char>& buffer) {
			writeVarInt((int32_t)tags.size(), buffer);

			for (const RegistryTag& registryTag : tags) {
				writeString(registryTag.registryName, buffer);
				writeVarInt((int32_t)registryTag.tags.size(), buffer);

				for (const RegistryTag::Tag& tag : registryTag.tags) {
					writeString(tag.name, buffer);
					writeVarInt((int32_t)tag.values.size(), buffer);

					for (const int32_t value : tag.values) {
						writeVarInt(value, buffer);
					}
				}
			}
        }
    };
};
