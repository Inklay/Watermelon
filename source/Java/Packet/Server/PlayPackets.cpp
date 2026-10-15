#include "Java/Packet/Server/PlayPackets.h"
#include "Java/Packet/Client/PlayPackets.h"

namespace Java {
	namespace Packet {
		namespace Server {
			namespace Play {
				static void setPlayerPositionAndRotation(std::shared_ptr<::Client> client, const Java::Packet::Header& header, Watermelon* server) {
				}

				static void confirmTeleportation(std::shared_ptr<::Client> client, const Java::Packet::Header& header, Watermelon* server) {
					uint32_t offset = 0;
					int32_t teleportId = Java::Packet::readVarInt(client, offset);
					Java::Packet::readDouble(client, offset); // x
					Java::Packet::readDouble(client, offset); // y
					Java::Packet::readDouble(client, offset); // z
					Java::Packet::readFloat(client, offset); // yaw
					Java::Packet::readFloat(client, offset); // pitch

					server->addOperationToQueue(client, [client, server, teleportId]() {
						Player* player = client->getPlayer();

						if (!player->finishTeleporting(teleportId)) {
							server->getLogger().log("Failed to teleport player " + player->getUsername(), "Java Network", Logger::WARN);
							Java::Packet::Client::Play::synchronizePlayerPosition(client, server);
						}
					});
				}

				void parse(std::shared_ptr<::Client> client, const Java::Packet::Header& header, Watermelon* server) {
					switch (header.packetId) {
					case (int32_t)Java::Packet::PacketId::PlayServer::CONFIRM_TELEPORTATION:
						return confirmTeleportation(client, header, server);
					case (int32_t)Java::Packet::PacketId::PlayServer::SET_PLAYER_POSITION_AND_ROTATION:
						return setPlayerPositionAndRotation(client, header, server);
					case (int32_t)Java::Packet::PacketId::PlayServer::CLIENT_TICK_END:
						return;
					}
				}
			};
		};
	};
};
