#pragma once

#include <mutex>
#include <memory>
#include <vector>
#include <stdexcept>
#include "Core/Client.h"
#include "Core/Logger.h"
#include "Java/Pack.h"
#include "Registry/Registry.h"
#include "Tag/RegistryTag.h"
#include "Player/ProfileProperty.h"

namespace Java {
	namespace Packet {

		struct parseError : public std::runtime_error {
			parseError(const std::string& what) : std::runtime_error(what) {}
		};

		enum Intent {
			STATUS = 1,
			LOGIN,
			TRANSFER 
		};

		enum class GameEvent {
			NO_RESPAWN = 0,
			BEGIN_RAIN,
			END_RAIN,
			CHANGE_GAME_MODE,
			WIN_GAME,
			DEMO_EVENT,
			PLAY_ARROW_HIT_SOUND,
			CHANGE_RAIN_LEVEL,
			CHANGE_THUNDER_LEVEL,
			PLAY_PUFFERFISH_STING_SOUND,
			PLAY_ELDER_GUARDIAN_JUMPSCARE,
			TOGGLE_IMMEDIATE_RESPAWN,
			TOGGLE_LIMITED_CRAFTING,
			START_WAITING_FOR_CHUNKS
		};

		enum HeightMapType {
			WORLD_SURFACE = 1,
			MOTION_BLOCKING = 4,
			MOTION_BLOCKING_NO_LEAVES = 5,
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
				MAP_DATA = 0x34,
				MERCHANT_OFFERS = 0x35,
				UPDATE_ENTITY_POSITION = 0x36,
				UPDATE_ENTITY_POSITION_AND_ROTATION = 0x37,
				MOVE_MINECART_ALON_TRACK = 0x38,
				UPDATE_ENTITY_ROTATION = 0x39,
				MOVE_VEHICLE = 0x3A,
				OPEN_BOOK = 0x3B,
				OPEN_SCREEN = 0x3C,
				OPEN_SIGN_EDITOR = 0x3D,
				PING = 0x3E,
				PING_RESPONSE = 0x3F,
				PLACE_GHOST_RECIPE = 0x40,
				PLAYER_ABILITIES = 0x41,
				PLAYER_CHAT_MESSAGE = 0x42,
				END_COMBAT = 0x43,
				ENTER_COMBAT = 0x44,
				COMBAT_DEATH = 0x45,
				PLAYER_INFO_REMOVE = 0x46,
				PLAYER_INFO_UPDATE = 0x47,
				LOOKT_AT = 0x48,
				SYNCHRONIZE_PLAYER_POSITION = 0x49,
				PLAYER_ROTATION = 0x4A,
				RECIPE_BOOK_ADD = 0x4B,
				RECIPE_BOOK_REMOVE = 0x4C,
				RECIPE_BOOK_SETTINGS = 0x4D,
				REMOVE_ENTITIES = 0x4E,
				REMOVE_ENTIT_EFFECT = 0x4F,
				RESET_SCORE = 0x50,
				REMOVE_RESOURCE_PACK = 0x51,
				ADD_RESOURCE_PACK = 0x52,
				POST_EFFECTS = 0x53,
				RESPAWN = 0x54,
				SET_HEAD_ROTATION = 0x55,
				UPDATE_SECTION_BLOCKS = 0x56,
				SELECT_ADVANCEMENTS_TAB = 0x57,
				SERVER_DATA = 0x58,
				SET_ACTION_BAR_TEXT = 0x59,
				SET_BORDER_CENTER = 0x5A,
				SET_BORDER_SIZE = 0x5B,
				SET_BORDER_LERP_SIZE = 0x5C,
				SET_BORDER_WARNING_DELAY = 0x5D,
				SET_BORDER_WARNING_DISTANCE = 0x5E,
				SET_CAMERA = 0x5F,
				SET_CENTER_CHUNK = 0x60,
				SET_RENDER_DISTANCE = 0x61,
				SET_CURSOR_ITEM = 0x62,
				SET_DEFAULT_SPAWN_POSITION = 0x63,
				DISPLAY_OBJECTIVE = 0x64,
				SET_ENTITY_METADATA = 0x65,
				LINK_ENTITIES = 0x66,
				SET_ENTITY_VELOCITY = 0x67,
				SET_EQUIPMENT = 0x68,
				SET_EXPERIENCE = 0x69,
				SET_HEALTH = 0x6A,
				SET_HELD_ITEM = 0x6B,
				UPDATE_OBJECTIVES = 0x6C,
				SET_PASSENGERS = 0x6D,
				SET_PLAYER_INVENTORY_SLOT = 0x6E,
				UPDATE_TEAMS = 0x6F,
				UPDATE_SCORE = 0x70,
				SET_SIMULATION_DISTANCE = 0x71,
				SET_SUBTITLE_TEXT = 0x72,
				UPDATE_TIME = 0x73,
				SET_TITLE_TEXT = 0x74,
				SET_TITLE_TEXT_ANIMATION_TIMES = 0x75,
				ENTITY_SOUND_EFFECT = 0x76,
				SOUND_EFFECT = 0x77,
				START_CONFIGURATION = 0x78,
				STOP_SOUND = 0x76,
				STORE_COOKIE = 0x7A,
				SWING_ANIMATION = 0x7B,
			};

			enum class PlayServer {
				CONFIRM_TELEPORTATION = 0x00,
				ATTACK = 0x01,
				QUERY_BLOCK_ENTITY_TAG = 0x02,
				BUNDLE_ITEM_SELECTED = 0x03,
				CHANGE_DIFFICULTY = 0x04,
				CHANGE_GAME_MODE = 0x05,
				ACKNOWLEDGE_MESSAGE = 0x06,
				CHAT_COMMAND = 0x07,
				SIGNED_CHAT_COMMAND = 0x08,
				CHAT_MESSAGE = 0x09,
				PLAYER_SESSION = 0x0A,
				CHUNK_BATCH_RECEIVED = 0x0B,
				CLIENT_STATUS = 0x0C,
				CLIENT_TICK_END = 0x0D,
				CLIENT_INFORMATION = 0x0E,
				COMMAND_SUGGESTION_REQUEST = 0x0F,
				ACKNOWLEDGE_CONFIGURATION = 0x10,
				CLICK_CONTAINER_BUTTON = 0x11,
				CLICK_CONTAINER = 0x12,
				CLOSE_CONTAINER = 0x13,
				CHANGE_CONTAINER_SLOT_STATE = 0x14,
				COOKIE_RESPONSE = 0x15,
				PLUGIN_MESSAGE = 0x16,
				DEBUG_SUBSCRIPTION_REQUEST = 0x17,
				EDIT_BOOK = 0x18,
				QUERY_ENTITY_TAG = 0x19,
				INTERACT = 0x1A,
				JIGSWA_GENERATE = 0x1B,
				KEEP_ALIVE = 0x1C,
				LOCK_DIFFICULTY = 0x1D,
				SET_PLAYER_POSITION = 0x1E,
				SET_PLAYER_POSITION_AND_ROTATION = 0x1F,
				SET_PLAYER_ROTATION  = 0x20,
				SET_PLAYER_MOVEMENT_FLAG = 0x21,
				MOVE_VEHICLE = 0x22,
				PADDLE_BOAT = 0x23,
				PICK_ITEM_FROM_BLOCK = 0x24,
				PICK_ITEM_FROM_ENTITY = 0x25,
				PING_REQUEST = 0x26,
				PLACE_RECIPE = 0x27,
				PLAYER_ABILITIES = 0x28,
				PLAYER_ACTION = 0x29,
				PLAYER_COMMAND = 0x2A,
				PLAYER_INPUT = 0x2B,
				PLAYER_LOADED = 0x2C,
				PONG = 0x2D,
				PUNCH = 0x2E,
				CHANGE_RECIPE_BOOK_SETTINGS = 0x2F,
			};
		};

		void checkEmpty(const std::vector<uint8_t>& packet, uint32_t& offset);

		std::string readString(const std::vector<uint8_t>& packet, uint32_t& offset);
		int32_t readVarInt(const std::vector<uint8_t>& packet, uint32_t& offset);
		bool readBool(const std::vector<uint8_t>& packet, uint32_t& offset);
		uint8_t readU8(const std::vector<uint8_t>& packet, uint32_t& offset);
		uint16_t readU16(const std::vector<uint8_t>& packet, uint32_t& offset);
		uint32_t readU32(const std::vector<uint8_t>& packet, uint32_t& offset);
		uint64_t readU64(const std::vector<uint8_t>& packet, uint32_t& offset);
		float readFloat(const std::vector<uint8_t>& packet, uint32_t& offset);
		double readDouble(const std::vector<uint8_t>& packet, uint32_t& offset);
		std::pair<uint64_t, uint64_t> readUUID(const std::vector<uint8_t>& packet, uint32_t& offset);
		std::vector<Pack> readPacks(const std::vector<uint8_t>& packet, uint32_t& offset);
		std::vector<uint8_t> readByteArray(const std::vector<uint8_t>& packet, uint32_t& offset);

		void writeString(const std::string& str, std::vector<uint8_t>& buffer);
		void writeVarInt(int32_t value, std::vector<uint8_t>& buffer);
		void writeBool(bool value, std::vector<uint8_t>& buffer);
		void write8(int8_t value, std::vector<uint8_t>& buffer);
		void writeU8(uint8_t value, std::vector<uint8_t>& buffer);
		void write16(int16_t value, std::vector<uint8_t>& buffer);
		void write32(int32_t value, std::vector<uint8_t>& buffer);
		void writeU32(uint32_t value, std::vector<uint8_t>& buffer);
		void write64(int64_t value, std::vector<uint8_t>& buffer);
		void writeFloat(float value, std::vector<uint8_t>& buffer);
		void writeDouble(double value, std::vector<uint8_t>& buffer);
		void writeUUID(std::pair<uint64_t, uint64_t>, std::vector<uint8_t>& buffer);
		void writePacks(std::vector<Pack>&, std::vector<uint8_t>& buffer);
		void writeRegistry(const Registry* registry, bool skipNbt, std::vector<uint8_t>& buffer);
		void writeNBT(const std::vector<uint8_t>& data, std::vector<uint8_t>& buffer);
		void writeTags(const std::vector<RegistryTag>& tags, std::vector<uint8_t>& buffer);
		void writeProfileProperties(const std::vector<Player::ProfileProperty>& properties, std::vector<uint8_t>& buffer);
		void writeString16(const std::string& value, std::vector<uint8_t>& buffer);
		void writeNBTTranslatedString(const std::string& value, std::vector<uint8_t>& buffer);
		void writeLpVec3(const Entity::Vec3& value, std::vector<uint8_t>& buffer);
		void writeAngle(float value, std::vector<uint8_t>& buffer);
		void writeRotation(const Entity::Rotation& value, std::vector<uint8_t>& buffer);
		void writePosition(const Entity::Vec3& value, std::vector<uint8_t>& buffer);
	};
}
