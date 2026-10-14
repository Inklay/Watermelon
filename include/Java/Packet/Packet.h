#pragma once

#include <mutex>
#include <memory>
#include <vector>
#include "Client.h"
#include "Logger.h"
#include "Java/Pack.h"
#include "Registry/Registry.h"
#include "Tag/RegistryTag.h"

namespace Java {
	namespace Packet {
		enum Intent {
			STATUS = 1,
			LOGIN = 2,
			TRANSFER = 3
		};

		enum Gamemode {
			SURVIVAL = 1,
			CREATIVE = 2,
			ADVENTURE = 3,
			SPECTATOR = 4
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

			enum class PlayClient {
				BUNDLE_DELIMITER = 0x00,
				SPANW_ENTITY = 0x01,
				ENTITY_ANIMATION = 0x02,
				AWARD_STATISTICS = 0x03,
				ACKNOWLEDGE_BLOCK_CHANGE = 0x04,
				SET_BLOCK_DETROY_STAGE = 0x05,
				BLOCK_ENTITY_DATA = 0x06,
				BLOCK_ACTION = 0x07,
				BLOCK_UPDATE = 0x08,
				BOSS_BAR = 0x09,
				CHANGE_DIFFICULTY = 0x0A,
				CHUNK_BATCH_FINISHED = 0x0B,
				CHUNK_BATCH_START = 0x0C,
				CHUNK_BIOMES = 0x0D,
				CLEAR_TITLES = 0x0E,
				COMMAND_SUGGESTIONS_RESPONSE = 0x0F,
				COMMANDS = 0x10,
				CLOSE_CONTAINER = 0x11,
				SET_CONAINER_CONTENT = 0x12,
				SET_CONAINER_PROPERTY = 0x13,
				SET_CONAINER_SLOT = 0x14,
				COOKIE_REQUEST = 0x15,
				SET_COOLDOWN = 0x16,
				CHAT_SUGGESTIONS = 0x17,
				PLUGIN_MESSAGE = 0x18,
				DAMAGE_EVENT = 0x19,
				DEBUG_BLOCK_VALUE = 0x1A,
				DEBUG_CHUNK_VALUE = 0x1B,
				DEBUG_ENTITY_VALUE = 0x1C,
				DEBUG_EVENT = 0x1D,
				DEBUG_SAMPLE = 0x1E,
				DEBUG_MESSAGE = 0x1F,
				DISCONNECT = 0x20,
				DISGUISED_CHAT_MESSAGE = 0x21,
				ENTITY_EVENT = 0x22,
				TELEPORT_ENTITY = 0x23,
				EXPLOSION = 0x24,
				ADD_TRANSIENT_BLOCK = 0x25,
				UNLOAD_CHUNK = 0x26,
				GAME_EVENT = 0x27,
				GAME_RULE_VALUES = 0x28,
				GAME_TEST_HIGHLIGHT_POSITION = 0x29,
				OPEN_HORSE_SCREEN = 0x2A,
				HURT_ANIMATION = 0x2B,
				INITIALIZE_WORLD_BORDER = 0x2C,
				KEEP_ALIVE = 0x2D,
				CHUNK_DATA_AND_UPDATE_LIGHT = 0x2E,
				WORLD_EVENT = 0x2F,
				PARTICLE = 0x30,
				UPDATE_LIGHT = 0x31,
				LOGIN = 0x32,
				LOW_DISK_SPACE_WARNING = 0x33,
			};
		};

		struct Header {
			int32_t length;
			int32_t packetId;
		};

		std::string readString(std::shared_ptr<::Client> client, uint32_t& offset);
		int32_t readVarInt(std::shared_ptr<::Client> client, uint32_t& offset);
		uint8_t readU8(std::shared_ptr<::Client> client, uint32_t& offset);
		uint16_t readU16(std::shared_ptr<::Client> client, uint32_t& offset);
		uint32_t readU32(std::shared_ptr<::Client> client, uint32_t& offset);
		int64_t read64(std::shared_ptr<::Client> client, uint32_t& offset);
		std::pair<uint64_t, uint64_t> readUUID(std::shared_ptr<::Client> client, uint32_t& offset);
		std::vector<Pack> readPacks(std::shared_ptr<::Client> client, uint32_t& offset);

		void writeString(const std::string& str, std::vector<char>& buffer);
		void writeVarInt(int32_t value, std::vector<char>& buffer);
		void writeBool(bool value, std::vector<char>& buffer);
		void write8(int8_t value, std::vector<char>& buffer);
		void writeU8(uint8_t value, std::vector<char>& buffer);
		void write16(int16_t value, std::vector<char>& buffer);
		void write32(int32_t value, std::vector<char>& buffer);
		void write64(int64_t value, std::vector<char>& buffer);
		void writeUUID(std::pair<uint64_t, uint64_t>, std::vector<char>& buffer);
		void writePacks(std::vector<Pack>&, std::vector<char>& buffer);
		void writeRegistry(const Registry* registry, bool skipNbt, std::vector<char>& buffer);
		void writeNBT(const std::vector<uint8_t>& data, std::vector<char>& buffer);
		void writeTags(const std::vector<RegistryTag>& tags, std::vector<char>& buffer);
	};
}
