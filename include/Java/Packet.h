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
			enum class HandshakeServer {
				HANDSHAKE = 0x00
			};

			enum class StatusClient {
				STATUS_RESPONSE = 0x00,
				PONG = 0x01
			};

			enum class StatusServer {
				STATUS_REQUEST = 0x00,
				PING = 0x01
			};

			enum class LoginServer{
				LOGIN_START = 0x00,
				ENCRYPTION_RESPONSE = 0x01,
				LOGIN_PLUGIN_RESPONSE = 0x02,
				LOGIN_ACKNOWLEDGED = 0x03,
				COOKIE_RESPONSE = 0x04
			};

			enum class LoginClient {
				DISCONNECT = 0x00,
				ENCRYPTION_REQUEST = 0x01,
				LOGIN_SUCCESS = 0x02,
				SET_COMPRESSION = 0x03,
				LOGIN_PLUGIN_REQUEST = 0x04,
				COOKIE_REQUEST = 0x05
			};

			enum class ConfigurationClient {
				COOKIE_REQUEST = 0x00,
				PLUGIN_MESSAGE = 0x01,
				DISCONNECT = 0x02,
				FINISH_CONFIGURATION = 0x03,
				KEEP_ALIVE = 0x04,
				PING = 0x05,
				RESET_CHAT = 0x06,
				REGISTRY_DATA = 0x07,
				REMOVE_RESOURCE_PACK = 0x08,
				ADD_RESOURCE_PACK = 0x09,
				POST_EFFECTS = 0x0A,
				STORE_COOKIE = 0x0B,
				TRANSFER = 0x0C,
				FEATURE_FLAGS = 0x0D,
				UPDATE_TAGS = 0x0E,
				KNOWN_PACKS = 0x0F,
				CUSTOM_REPORT_DETAIL = 0x10,
				SERVER_LINK = 0x11,
				CLEAR_DIALOG = 0x12,
				SHOW_DIALOG = 0x13,
				CODE_OF_CONDUCT = 0x14
			};

			enum class ConfigurationServer {
				CLIENT_INFORMATION = 0x00,
				COOKIE_RESPONSE = 0x01,
				PLUGIN_MESSAGE = 0x02,
				ACKNOWLEDGE_FINISH_CONFIGURATION = 0x03,
				KEEP_ALIVE = 0x04,
				PONG = 0x05,
				RESOURCE_PACK_RESPONSE = 0x06,
				KNOWN_PACKS = 0x07,
				CUSTOM_CLICK_ACTION = 0x08,
				ACCEPT_CODE_OF_CONDUCT = 0x09
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
