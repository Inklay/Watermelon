#include "Java/Packet/Packet.h"
#include "Registry/RegistryManager.h"
#include "Java/Packet/Client/StatusPackets.h"

void buildPlayerInfoUpdatePacket(const std::vector<Player::InfoAction>& actions, const std::vector<const Player::Player*>& players, std::vector<uint8_t>& buffer) {
	Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::PlayClient::PLAYER_INFO_UPDATE, buffer);

	uint8_t actionBitMap = 0;
	for (const Player::InfoAction action : actions) {
		actionBitMap |= (uint8_t)action;
	}

	Java::Packet::writeU8(actionBitMap, buffer);
	Java::Packet::writeVarInt((int32_t)players.size(), buffer);

	for (const Player::Player* player : players) {
		Java::Packet::writeUUID(player->getUUID(), buffer);

		if (actionBitMap & (uint8_t)Player::InfoAction::ADD_PLAYER) {
			Java::Packet::writeString(player->getUsername(), buffer);
			Java::Packet::writeProfileProperties(player->getProfileProperties(), buffer);
		}

		if (actionBitMap & (uint8_t)Player::InfoAction::INITIALIZE_CHAT) {
			if (player->hasChatSignature()) {
				Java::Packet::writeBool(true, buffer);
				Java::Packet::writeBool(false, buffer); // FIXME chat session id
				Java::Packet::writeBool(false, buffer); // FIXME public key expiry time
				Java::Packet::writeBool(false, buffer); // FIXME encoded public key
				Java::Packet::writeBool(false, buffer); // FIXME public key signature
			} else {
				Java::Packet::writeBool(false, buffer);
			}
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
}

namespace Java {
	namespace Packet {
		namespace Client {
			namespace Play {
				void disconnect(const std::shared_ptr<::Client>& client, const std::string& reason) {
					std::vector<uint8_t> buffer;

					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::PlayClient::DISCONNECT, buffer);
					Java::Packet::writeNBTTranslatedString(reason, buffer);
					client->sendData(buffer);
				}

				void playerInfoUpdateOne(const std::vector<Player::InfoAction>& actions, const std::vector<const Player::Player*>& players, const std::shared_ptr<::Client>& client) {
					std::vector<uint8_t> buffer;
					buildPlayerInfoUpdatePacket(actions, players, buffer);

					client->sendData(buffer);
				}

				void playerInfoUpdateAll(Watermelon* server, const std::vector<Player::InfoAction>& actions, const std::vector<const Player::Player*>& players, const std::shared_ptr<::Client>& sendingClient) {
					std::vector<uint8_t> buffer;
					buildPlayerInfoUpdatePacket(actions, players, buffer);

					for (auto& client : server->getClients()) {
						if (client == sendingClient) {
							continue;
						}

						if (client->getStatus() == ::Client::Status::PLAY) {
							std::vector<uint8_t> clientBuffer = buffer;
							client->sendData(clientBuffer);
						}
					}
				}

				void synchronizePlayerPosition(const std::shared_ptr<::Client>& client) {
					std::vector<uint8_t> buffer;
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

					client->sendData(buffer);
				}

				void gameEvent(const std::shared_ptr<::Client>& client, GameEvent event, float data) {
					std::vector<uint8_t> buffer;

					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::PlayClient::GAME_EVENT, buffer);
					Java::Packet::writeU8((int8_t)event, buffer);
					Java::Packet::writeFloat(data, buffer);

					client->sendData(buffer);
				}

				void setCenterChunk(const std::shared_ptr<::Client>& client) {
					std::vector<uint8_t> buffer;
					Player::Player* player = client->getPlayer();

					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::PlayClient::SET_CENTER_CHUNK, buffer);
					Java::Packet::writeVarInt((int32_t)(std::floor(player->getPosition().x / 16)), buffer);
					Java::Packet::writeVarInt((int32_t)(std::floor(player->getPosition().z / 16)), buffer);

					client->sendData(buffer);
				}

				void chunkDataAndUpdateLight(const std::shared_ptr<::Client>& client, int x, int z) {
					const int dimensionHeight = 384; // FIXME needs to read height from register
					const int sectionNumber = dimensionHeight / 16;
					const int heightmapArrayCount = 37;
					std::vector<uint8_t> buffer;
					std::vector<uint8_t> sectionBuffer;

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

					client->sendData(buffer);
				}

				void login(const std::shared_ptr<::Client>& client, Watermelon* server) {
					std::vector<uint8_t> buffer;
					const Registry* dimensionReg = RegistryManager::getInstance().getRegistry("minecraft:dimension_type");
					Player::Player* player = client->getPlayer();

					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::PlayClient::LOGIN, buffer);
					Java::Packet::write32(player->getEntityId(), buffer);
					Java::Packet::writeBool(server->getConfig()["hardcore"].value_or(false), buffer);
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

					if (server->getConfig()["force_gamemode"].value_or(false)) {
						Java::Packet::writeVarInt(server->getDefaultGamemode(), buffer);
					} else {
						Java::Packet::writeVarInt(player->getGamemode(), buffer);
					}

					Java::Packet::writeVarInt(0, buffer); // FIXME previous game mode
					Java::Packet::writeBool(false, buffer); // FIXME is debug
					Java::Packet::writeBool(false, buffer); // FIXME is flat
					Java::Packet::writeBool(false, buffer); // FIXME has death location
					Java::Packet::writeVarInt(0, buffer);
					Java::Packet::writeVarInt(63, buffer);
					Java::Packet::writeBool(server->isOnlineMode(), buffer);
					Java::Packet::writeBool(false, buffer); // FIXME enforce secure chat

					client->sendData(buffer);
				}

				void keepAlive(const std::shared_ptr<::Client>& client, uint64_t keepAliveid) {
					std::vector<uint8_t> buffer;

					Java::Packet::writeVarInt(static_cast<uint32_t>(Java::Packet::PacketId::PlayClient::KEEP_ALIVE), buffer);
					Java::Packet::write64(keepAliveid, buffer);

					client->sendData(buffer);
				}

				void spawnEntity(const std::shared_ptr<::Client>& client, Entity* entity) {
					std::vector<uint8_t> buffer;

					Java::Packet::writeVarInt(static_cast<uint32_t>(Java::Packet::PacketId::PlayClient::SPANW_ENTITY), buffer);
					Java::Packet::writeVarInt(entity->getEntityId(), buffer);
					Java::Packet::writeUUID(entity->getUUID(), buffer);
					Java::Packet::writeVarInt(static_cast<uint32_t>(entity->getType()), buffer);
					Java::Packet::writePosition(entity->getPosition(), buffer);
					Java::Packet::writeLpVec3(entity->getVelocity(), buffer);
					Java::Packet::writeRotation(entity->getRotation(), buffer);
					Java::Packet::writeVarInt(entity->getProtocolData(), buffer);

					client->sendData(buffer);
				}

				void removeEntities(const std::shared_ptr<::Client>& client, std::vector<uint32_t> ids) {
					std::vector<uint8_t> buffer;

					Java::Packet::writeVarInt(static_cast<uint32_t>(Java::Packet::PacketId::PlayClient::REMOVE_ENTITIES), buffer);
					Java::Packet::writeVarInt(static_cast<uint32_t>(ids.size()), buffer);
					
					for (uint32_t id : ids) {
						Java::Packet::writeVarInt(id, buffer);
					}

					client->sendData(buffer);

				}
			};
		};
	};
};
