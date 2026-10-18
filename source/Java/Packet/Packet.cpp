#include "Java/Packet/Packet.h"
#include <iostream>
#include "Utils/Byte.h"

namespace Java {
    namespace Packet {
        void checkEmpty(const std::vector<uint8_t>& packet, uint32_t& offset) {
            if (offset != packet.size()) {
                throw parseError( "Unexpected trailing bytes");
            }
        }

        std::string readString(const std::vector<uint8_t>& packet, uint32_t& offset) {
            int32_t length = readVarInt(packet, offset);

            if (length < 0 || length > 2000000) {
                throw parseError("Invalid string: invalid lenght");
            }

            if (offset > packet.size() || static_cast<size_t>(length) > packet.size() - offset) {
                throw parseError("Invalid string: unexpected end of packet");
            }

            std::string result(packet.begin() + offset, packet.begin() + offset + length);
            offset += static_cast<uint32_t>(length);
            return result;
        }

        int32_t readVarInt(const std::vector<uint8_t>& packet, uint32_t& offset) {
            uint32_t value = 0;

            for (int i = 0; i < 5; ++i) {
                if (offset >= packet.size()) {
                    throw parseError("Invalid VarInt: unexpected end of packet");
                }

                const uint8_t byte = packet[offset++];

                if (i == 4 && (byte & 0xF0) != 0) {
                    throw parseError("Invalid VarInt: overflow");
                }

                value |= static_cast<uint32_t>(byte & 0x7F) << (7 * i);

                if ((byte & 0x80) == 0) {
                    return static_cast<int32_t>(value);
                }
            }

            throw parseError("Invalid VarInt: too long");
        }

        bool readBool(const std::vector<uint8_t>& packet, uint32_t& offset) {
            return readU8(packet, offset);
        }

        uint8_t readU8(const std::vector<uint8_t>& packet, uint32_t& offset) {
            if (offset >= packet.size()) {
                throw parseError("Invalid u8: unexpected end of packet");
            }

            return packet[offset++];
        }

        uint16_t readU16(const std::vector<uint8_t>& packet, uint32_t& offset) {
            if (offset > packet.size() || packet.size() - offset < 2) {
                throw parseError("Invalid u16: unexpected end of packet");
            }

            uint16_t value = 0;

            for (int i = 0; i < 2; ++i) {
                value = (value << 8) | packet[offset++];
            }

            return value;
        }

        uint32_t readU32(const std::vector<uint8_t>& packet, uint32_t& offset) {
            if (offset > packet.size() || packet.size() - offset < 4) {
                throw parseError("Invalid u32: unexpected end of packet");
            }

            uint32_t value = 0;

            for (int i = 0; i < 4; ++i) {
                value = (value << 8) | packet[offset++];
            }

            return value;
        }

        uint64_t readU64(const std::vector<uint8_t>& packet, uint32_t& offset) {
            if (offset > packet.size() || packet.size() - offset < 8) {
                throw parseError("Invalid u64: unexpected end of packet");
            }

            uint64_t value = 0;

            for (int i = 0; i < 8; ++i) {
                value = (value << 8) | packet[offset++];
            }

            return value;
        }

        float readFloat(const std::vector<uint8_t>& packet, uint32_t& offset) {
            uint32_t bits = readU32(packet, offset);
            float value;

            std::memcpy(&value, &bits, sizeof(value));

            return value;
        }

        double readDouble(const std::vector<uint8_t>& packet, uint32_t& offset) {
            uint64_t bits = readU64(packet, offset);
            double value;

            std::memcpy(&value, &bits, sizeof(value));

            return value;
        }

        std::pair<uint64_t, uint64_t> readUUID(const std::vector<uint8_t>& packet, uint32_t& offset) {
            uint64_t a = readU64(packet, offset);
            uint64_t b = readU64(packet, offset);

            return std::make_pair(a, b);
        }

        std::vector<Pack> readPacks(const std::vector<uint8_t>& packet, uint32_t& offset) {
            int32_t size = readVarInt(packet, offset);
            constexpr int32_t MAX_PACKS = 1024;

            if (size < 0 || size > MAX_PACKS) {
                throw parseError("Invalid packs: invalid size");
            }

            std::vector<Pack> packs;
            std::string nameSpace, id, version;
            packs.reserve(size);

            for (int i = 0; i < size; i++) {
                nameSpace = readString(packet, offset);
                id = readString(packet, offset);
                version = readString(packet, offset);

                packs.emplace_back(nameSpace, id, version);
            }

            return packs;
        }

        std::vector<uint8_t> readByteArray(const std::vector<uint8_t>& packet, uint32_t& offset) {
            int32_t size = readVarInt(packet, offset);
            constexpr int32_t MAX_ARRAY_SIZE = 8192;

            if (size < 0 || size > MAX_ARRAY_SIZE) {
                throw parseError("Invalid byte array: invalid size");
            }

            std::vector<uint8_t> data;
            data.reserve(size);

            for (int i = 0; i < size; i++) {
                data.emplace_back(readU8(packet, offset));
            }

            return data;
        }

        void writeString(const std::string& str, std::vector<uint8_t>& buffer) {
            writeVarInt((int)str.size(), buffer);
            for (char c : str) {
                buffer.push_back(c);
            }
        }

        void writeVarInt(int32_t value, std::vector<uint8_t>& buffer) {
            uint32_t unsignedValue = static_cast<uint32_t>(value);

            do {
                uint8_t temp = unsignedValue & 0x7F;
                unsignedValue >>= 7;

                if (unsignedValue != 0) {
                    temp |= 0x80;
                }

                buffer.push_back(static_cast<uint8_t>(temp));
            } while (unsignedValue != 0);
        }

        void writeBool(bool value, std::vector<uint8_t>& buffer) {
            write8(value ? 0x01 : 0x00, buffer);
        }

        void writeU8(uint8_t value, std::vector<uint8_t>& buffer) {
            buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(&value), reinterpret_cast<const uint8_t*>(&value) + sizeof(value));
        }

        void write8(int8_t value, std::vector<uint8_t>& buffer) {
            buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(&value), reinterpret_cast<const uint8_t*>(&value) + sizeof(value));
        }

        void write16(int16_t value, std::vector<uint8_t>& buffer) {
            int16_t networkValue = Byte::toBigEndian(value);
            buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(&networkValue), reinterpret_cast<const uint8_t*>(&networkValue) + sizeof(networkValue));
        }

        void write32(int32_t value, std::vector<uint8_t>& buffer) {
            int32_t networkValue = Byte::toBigEndian(value);
            buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(&networkValue), reinterpret_cast<const uint8_t*>(&networkValue) + sizeof(networkValue));
        }

        void write64(int64_t value, std::vector<uint8_t>& buffer) {
            int64_t networkValue = Byte::toBigEndian(value);
            buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(&networkValue), reinterpret_cast<const uint8_t*>(&networkValue) + sizeof(networkValue));
        }

        void writeFloat(float value, std::vector<uint8_t>& buffer) {
            uint32_t networkValue = Byte::toBigEndian(std::bit_cast<uint32_t>(value));
            buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(&networkValue), reinterpret_cast<const uint8_t*>(&networkValue) + sizeof(networkValue));
        }

        void writeDouble(double value, std::vector<uint8_t>& buffer) {
            uint64_t networkValue = Byte::toBigEndian(std::bit_cast<uint64_t>(value));
            buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(&networkValue), reinterpret_cast<const uint8_t*>(&networkValue) + sizeof(networkValue));
        }

        void writeUUID(std::pair<uint64_t, uint64_t> uuid, std::vector<uint8_t>& buffer) {
            write64(uuid.first, buffer);
            write64(uuid.second, buffer);
        }

        void writePacks(std::vector<Pack>& packs, std::vector<uint8_t>& buffer) {
            writeVarInt((int32_t)packs.size(), buffer);

            for (const Pack& i : packs) {
                writeString(i.nameSpace, buffer);
                writeString(i.id, buffer);
                writeString(i.version, buffer);
            }
        }

        void writeRegistry(const Registry* registry, bool skipNbt, std::vector<uint8_t>& buffer) {
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

        void writeNBT(const std::vector<uint8_t>& data, std::vector<uint8_t>& buffer) {
            for (const uint8_t byte : data) {
                writeU8(byte, buffer);
            }
        }

        void writeTags(const std::vector<RegistryTag>& tags, std::vector<uint8_t>& buffer) {
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
