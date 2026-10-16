#include "Java/Packet/Packet.h"
#include "Registry/RegistryManager.h"
#include "Java/Packet/Client/StatusPackets.h"

namespace Java {
	namespace Packet {
		namespace Client {
			namespace Play {
				void playerInfoUpdate(std::shared_ptr<::Client> client, Watermelon* server, const std::vector<Player::InfoAction>& actions, const std::vector<Player*> players) {
					std::vector<char> buffer;

					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::PlayClient::PLAYER_INFO_UPDATE, buffer);

					uint8_t actionBitMap = 0;
					for (const Player::InfoAction action : actions) {
						actionBitMap |= (uint8_t)action;
					}

					Java::Packet::writeU8(actionBitMap, buffer);
					Java::Packet::writeVarInt((int32_t)players.size(), buffer);

					for (Player* player: players) {
						Java::Packet::writeUUID(player->getUUID(), buffer);

						if (actionBitMap & (uint8_t)Player::InfoAction::ADD_PLAYER) {
							Java::Packet::writeString(player->getUsername(), buffer);
							Java::Packet::writeVarInt(0, buffer); // FIXME Game profile
						}

						if (actionBitMap & (uint8_t)Player::InfoAction::INITIALIZE_CHAT) {
							Java::Packet::writeBool(false, buffer); // FIXME chat session id
							Java::Packet::writeBool(false, buffer); // FIXME public key expiry time
							Java::Packet::writeBool(false, buffer); // FIXME encoded public key
							Java::Packet::writeBool(false, buffer); // FIXME public key signature
						}

						if (actionBitMap & (uint8_t)Player::InfoAction::UPDATE_GAME_MODE) {
							Java::Packet::writeVarInt(player->getGamemode(), buffer);
						}

						if (actionBitMap & (uint8_t)Player::InfoAction::UPDATE_LISTED) {
							Java::Packet::writeBool(true, buffer); // FIXME listed
						}

						if (actionBitMap & (uint8_t)Player::InfoAction::UPDATE_LATENCY) {
							Java::Packet::writeVarInt(0, buffer); // FIXME ping
						}

						if (actionBitMap & (uint8_t)Player::InfoAction::UPDATE_DISPLAY_NAME) {
							Java::Packet::writeBool(false, buffer); // FIXME display name
						}

						if (actionBitMap & (uint8_t)Player::InfoAction::UPDATE_LIST_PRIORITY) {
							Java::Packet::writeVarInt(0, buffer); // FIXME list priority
						}

						if (actionBitMap & (uint8_t)Player::InfoAction::UPDATE_HAT) {
							Java::Packet::writeBool(true, buffer); // FIXME visible
						}
					}

					for (auto& client : server->getClients()) {
						server->sendDataToClient(client, buffer);
					}
				}

				void synchronizePlayerPosition(std::shared_ptr<::Client> client, Watermelon* server) {
					std::vector<char> buffer;
					int32_t teleportId = client->getPlayer()->teleport({ 0, 0, 0 }, { 0, 0 });

					if (teleportId == -1) {
						return;
					}

					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::PlayClient::SYNCHRONIZE_PLAYER_POSITION, buffer);
					Java::Packet::writeVarInt(teleportId, buffer);
					Java::Packet::writeDouble(0, buffer); // FIXME x
					Java::Packet::writeDouble(0, buffer); // FIXME y
					Java::Packet::writeDouble(0, buffer); // FIXME z
					Java::Packet::writeDouble(0, buffer);
					Java::Packet::writeDouble(0, buffer);
					Java::Packet::writeDouble(0, buffer);
					Java::Packet::writeFloat(0, buffer); // FIXME yaw
					Java::Packet::writeFloat(0, buffer); // FIXME pitch
					Java::Packet::write32(0, buffer);

					server->sendDataToClient(client, buffer);
				}

				void gameEvent(std::shared_ptr<::Client> client, Watermelon* server, GameEvent event, float data) {
					std::vector<char> buffer;

					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::PlayClient::GAME_EVENT, buffer);
					Java::Packet::writeU8(event, buffer);
					Java::Packet::writeFloat(data, buffer);

					server->sendDataToClient(client, buffer);
				}

				void setCenterChunk(std::shared_ptr<::Client> client, Watermelon* server) {
					std::vector<char> buffer;
					Player* player = client->getPlayer();

					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::PlayClient::SET_CENTER_CHUNK, buffer);
					Java::Packet::writeVarInt((int32_t)(player->getPosition().x / 16), buffer);
					Java::Packet::writeVarInt((int32_t)(player->getPosition().z / 16), buffer);

					server->sendDataToClient(client, buffer);
				}

				void chunkDataAndUpdateLight(std::shared_ptr<::Client> client, Watermelon* server, int x, int z) {
					const int dimensionHeight = 384; // FIXME needs to read height from register
					const int sectionNumber = dimensionHeight / 16;
					const int heightmapArrayCount = 37;
					std::vector<char> buffer;
					std::vector<char> sectionBuffer;

					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::PlayClient::CHUNK_DATA_AND_UPDATE_LIGHT, buffer);
					Java::Packet::write32(x, buffer);
					Java::Packet::write32(z, buffer);
					Java::Packet::writeVarInt(1, buffer); // FIXME heightmaps count
					Java::Packet::writeVarInt(HeightMapType::WORLD_SURFACE, buffer); // FIXME heightmaps type
					Java::Packet::writeVarInt(heightmapArrayCount, buffer);

					for (int i = 0; i < heightmapArrayCount; i++) {
						Java::Packet::write64(0, buffer); // FIXME heightmap data (9 bits per column so 7 columns per int64)
					}

					for (int sectionY = 0; sectionY < sectionNumber; ++sectionY) {
						Java::Packet::write16(0, sectionBuffer); // FIXME block count
						Java::Packet::write16(0, sectionBuffer); // FIXME fluid count
						Java::Packet::writeU8(0, sectionBuffer); // FIXME block palette formats
						Java::Packet::writeVarInt(0, sectionBuffer); // FIXME block palette
						Java::Packet::writeU8(0, sectionBuffer); // FIXME biome palette formats
						Java::Packet::writeVarInt(0, sectionBuffer); // FIXME biome palette
					}

					Java::Packet::writeVarInt((int)sectionBuffer.size(), buffer);
					buffer.insert(buffer.end(), sectionBuffer.begin(), sectionBuffer.end());
					Java::Packet::writeVarInt(0, buffer); // FIXME block entities count

					Java::Packet::writeVarInt(0, buffer); // FIXME sky light mask lenght
					Java::Packet::writeVarInt(0, buffer); // FIXME block light mask lenght
					Java::Packet::writeVarInt(8, buffer); // FIXME empty sky light mask lenght
					Java::Packet::write64(0x3FFFFFF, buffer); // FIXME sky light mask
					Java::Packet::writeVarInt(8, buffer); // FIXME empty block light mask lenght
					Java::Packet::write64(0x3FFFFFF, buffer); // FIXME empty block light mask
					Java::Packet::writeVarInt(0, buffer); // FIXME sky light arrays
					Java::Packet::writeVarInt(0, buffer); // FIXME block light arrays

					server->sendDataToClient(client, buffer);
				}

				void login(std::shared_ptr<::Client> client, Watermelon* server) {
					std::vector<char> buffer;
					const Registry* dimensionReg = RegistryManager::getInstance().getRegistry("minecraft:dimension_type");
					Player* player = client->getPlayer();

					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::PlayClient::LOGIN, buffer);
					Java::Packet::write32(client->getPlayer()->getEntityId(), buffer);
					Java::Packet::writeBool(false, buffer); // FIXME is hardcore
					Java::Packet::writeVarInt((int32_t)dimensionReg->entries.size(), buffer);

					for (const Registry::Entry& entry : dimensionReg->entries) {
						Java::Packet::writeString(entry.textId, buffer);
					}

					Java::Packet::writeVarInt(server->getConfig()["networking"]["java"]["max_players"].value_or(10), buffer);
					Java::Packet::writeVarInt(server->getConfig()["networking"]["java"]["view_distance"].value_or(16), buffer);
					Java::Packet::writeVarInt(server->getConfig()["networking"]["java"]["simulation_distance"].value_or(10), buffer);
					Java::Packet::writeBool(false, buffer);
					Java::Packet::writeBool(true, buffer); // FIXME enable respawn screen
					Java::Packet::writeBool(false, buffer);
					Java::Packet::writeVarInt(dimensionReg->entries.at(0).protocolId, buffer); // FIXME dimension type
					Java::Packet::writeString(dimensionReg->entries.at(0).textId, buffer); // FIXME dimension name
					Java::Packet::write64(0, buffer); // FIXME hashed seed
					Java::Packet::writeVarInt(player->getGamemode(), buffer);
					Java::Packet::writeVarInt(0, buffer); // FIXME previous game mode
					Java::Packet::writeBool(false, buffer); // FIXME is debug
					Java::Packet::writeBool(false, buffer); // FIXME is flat
					Java::Packet::writeBool(false, buffer); // FIXME has death location
					Java::Packet::writeVarInt(0, buffer);
					Java::Packet::writeVarInt(63, buffer);
					Java::Packet::writeBool(server->getConfig()["networking"]["java"]["online_mode"].value_or(true), buffer);
					Java::Packet::writeBool(false, buffer); // FIXME enforce secure chat

					server->sendDataToClient(client, buffer);
				}
			};
		};
	};
};
