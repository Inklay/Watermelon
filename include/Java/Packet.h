#pragma once

#include <mutex>
#include <memory>
#include <vector>
#include "../Client.h"
#include "../Logger.h"

namespace Java {
	namespace Packet {
		enum Intent {
			STATUS = 1,
			LOGIN = 2,
			TRANSFER = 3
		};
		namespace PacketId {
			enum HandshakeServer {
				HANDSHAKE = 0x00
			};

			enum StatusClient {
				STATUS_RESPONSE = 0x00,
				PONG = 0x01
			};

			enum StatusServer {
				STATUS_REQUEST = 0x00,
				PING = 0x01
			};

			enum LoginServer{
				LOGIN_START = 0x00,
				ENCRYPTION_RESPONSE = 0x01,
				LOGIN_PLUGIN_RESPONSE = 0x02,
				LOGIN_AKNOWLEDGED = 0x03,
				COOKIE_RESPONSE = 0x04
			};

			enum LoginClient {
				DISCONNECT = 0x00,
				ENCRYPTION_REQUEST = 0x01,
				LOGIN_SUCCESS = 0x02,
				SET_COMPRESSION = 0x03,
				LOGIN_PLUGIN_REQUEST = 0x04,
				COOKIE_REQUEST = 0x05
			};
		};

		struct Header {
			int32_t length;
			int32_t packetId;
		};

		std::string readString(std::shared_ptr<Client> client, uint32_t& offset);
		int32_t readVarInt(std::shared_ptr<Client> client, uint32_t& offset);
		uint8_t readU8(std::shared_ptr<Client> client, uint32_t& offset);
		uint16_t readU16(std::shared_ptr<Client> client, uint32_t& offset);
		uint32_t readU32(std::shared_ptr<Client> client, uint32_t& offset);
		int64_t read64(std::shared_ptr<Client> client, uint32_t& offset);
		std::pair<uint64_t, uint64_t> readUUID(std::shared_ptr<Client> client, uint32_t& offset);

		void writeString(const std::string& str, std::vector<char>& buffer);
		void writeVarInt(int32_t value, std::vector<char>& buffer);
		void write8(int8_t value, std::vector<char>& buffer);
		void write16(int16_t value, std::vector<char>& buffer);
		void write32(int32_t value, std::vector<char>& buffer);
		void write64(int64_t value, std::vector<char>& buffer);
		void writeUUID(std::pair<uint64_t, uint64_t>, std::vector<char>& buffer);
	};
}
