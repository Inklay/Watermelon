#include "Java/Packet/Server/PlayPackets.h"
#include "Java/Packet/Client/PlayPackets.h"

namespace Java {
	namespace Packet {
		namespace Server {
			namespace Play {
				static void setPlayerPositionAndRotation(const std::shared_ptr<::Client>& client, Watermelon* server, const std::vector<uint8_t>& packet) {
					uint32_t offset = 0;
					double x = Java::Packet::readDouble(packet, offset);
					double y = Java::Packet::readDouble(packet, offset);
					double z = Java::Packet::readDouble(packet, offset);
					float yaw = Java::Packet::readFloat(packet, offset);
					float pitch = Java::Packet::readFloat(packet, offset);
					Java::Packet::readU8(packet, offset); // FIXME flags
					Java::Packet::checkEmpty(packet, offset);

					server->addOperationToQueue(client, [client, x, y, z, yaw, pitch]() {
						Player::Player* player = client->getPlayer();
						
						if (!player->setPosition({ x, y, z })) {
							return;
						}
						player->setRotation({ yaw, pitch });
					});
				}

				static void confirmTeleportation(const std::shared_ptr<::Client>& client,Watermelon* server, const std::vector<uint8_t>& packet) {
					uint32_t offset = 0;
					int32_t teleportId = Java::Packet::readVarInt(packet, offset);
					Java::Packet::readDouble(packet, offset); // x
					Java::Packet::readDouble(packet, offset); // y
					Java::Packet::readDouble(packet, offset); // z
					Java::Packet::readFloat(packet, offset); // yaw
					Java::Packet::readFloat(packet, offset); // pitch
					Java::Packet::checkEmpty(packet, offset);

					server->addOperationToQueue(client, [client, server, teleportId]() {
						Player::Player* player = client->getPlayer();

						if (!player->finishTeleporting(teleportId)) {
							server->getLogger().log("Failed to teleport player " + player->getUsername(), "Network", Logger::WARN);
							Java::Packet::Client::Play::synchronizePlayerPosition(client);
						}
					});
				}

				static void playerSession(const std::shared_ptr<::Client>& client, Watermelon* server, const std::vector<uint8_t>& packet) {
					uint32_t offset = 0;
					Player::ChatSession chatSession = {
						Java::Packet::readUUID(packet, offset),
						Java::Packet::readU64(packet, offset),
						Java::Packet::readByteArray(packet, offset),
						Java::Packet::readByteArray(packet, offset)
					};
					Java::Packet::checkEmpty(packet, offset);

					server->addOperationToQueue(client, [client, chatSession = std::move(chatSession)]() {
						client->getPlayer()->setChatSession(std::move(chatSession));
					});
				}

				static void playerLoaded(const std::shared_ptr<::Client>& client, Watermelon* server, const std::vector<uint8_t>& packet) {
					uint32_t offset = 0;
					Java::Packet::checkEmpty(packet, offset);

					server->addOperationToQueue(client, [client]() {
						client->getPlayer()->setLoading(false);
					});
				}

				static void clientTickEnd(const std::shared_ptr<::Client>& client, Watermelon* server, const std::vector<uint8_t>& packet) {
					uint32_t offset = 0;
					Java::Packet::checkEmpty(packet, offset);

					server->addOperationToQueue(client, [client]() {
						client->getPlayer()->clientTickEnd();
					});
				}

				static void setPlayerPosition(const std::shared_ptr<::Client>& client, Watermelon* server, const std::vector<uint8_t>& packet) {
					uint32_t offset = 0;
					double x = Java::Packet::readDouble(packet, offset);
					double y = Java::Packet::readDouble(packet, offset);
					double z = Java::Packet::readDouble(packet, offset);
					Java::Packet::readU8(packet, offset); // FIXME flags
					Java::Packet::checkEmpty(packet, offset);

					server->addOperationToQueue(client, [client, x, y, z]() {
						Player::Player* player = client->getPlayer();

						if (!player->setPosition({ x, y, z })) {
							return;
						}
						
					});
				}

				static void playerInput(const std::shared_ptr<::Client>& client, Watermelon* server, const std::vector<uint8_t>& packet) {
					uint32_t offset = 0;
					uint8_t inputFlag = Java::Packet::readU8(packet, offset);
					Java::Packet::checkEmpty(packet, offset);

					server->addOperationToQueue(client, [client, inputFlag]() {
						client->getPlayer()->setInputFlag(inputFlag);
					});
				}

				static void playerAbilities(const std::shared_ptr<::Client>& client, Watermelon* server, const std::vector<uint8_t>& packet) {
					uint32_t offset = 0;
					uint8_t flags = Java::Packet::readU8(packet, offset);
					Java::Packet::checkEmpty(packet, offset);

					server->addOperationToQueue(client, [client, flags]() {
						client->getPlayer()->setFlying(flags & 0x02);
					});
				}

				static void keepAlive(const std::shared_ptr<::Client>& client, Watermelon* server, const std::vector<uint8_t>& packet) {
					uint32_t offset = 0;
					uint64_t keepAliveId = Java::Packet::readU64(packet, offset);
					Java::Packet::checkEmpty(packet, offset);

					server->addOperationToQueue(client, [client, keepAliveId]() {
						client->validateKeepAlive(keepAliveId);
					});
				}

				void parse(const std::shared_ptr<::Client>& client, int32_t packetId, Watermelon* server, const std::vector<uint8_t>& packet) {
					switch (packetId) {
					case (int32_t)Java::Packet::PacketId::PlayServer::CONFIRM_TELEPORTATION:
						return confirmTeleportation(client, server, packet);
					case (int32_t)Java::Packet::PacketId::PlayServer::SET_PLAYER_POSITION:
						return setPlayerPosition(client, server, packet);
					case (int32_t)Java::Packet::PacketId::PlayServer::SET_PLAYER_POSITION_AND_ROTATION:
						return setPlayerPositionAndRotation(client, server, packet);
					case (int32_t)Java::Packet::PacketId::PlayServer::PLAYER_SESSION:
						return playerSession(client, server, packet);
					case (int32_t)Java::Packet::PacketId::PlayServer::CLIENT_TICK_END:
						return clientTickEnd(client, server, packet);
					case (int32_t)Java::Packet::PacketId::PlayServer::PLAYER_LOADED:
						return playerLoaded(client, server, packet);
					case (int32_t)Java::Packet::PacketId::PlayServer::PLAYER_INPUT:
						return playerInput(client, server, packet);
					case (int32_t)Java::Packet::PacketId::PlayServer::PLAYER_ABILITIES:
						return playerAbilities(client, server, packet);
					case (int32_t)Java::Packet::PacketId::PlayServer::KEEP_ALIVE:
						return keepAlive(client, server, packet);
					default:
						server->getLogger().log("Unknown packet ID: " + std::to_string(packetId), "Network", Logger::ERR);
						server->disconnectClient(client.get());
					}
				}
			};
		};
	};
};
