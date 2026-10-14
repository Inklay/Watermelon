#include "Java/Packet/Server/PlayPackets.h"
#include "Java/Packet/Client/PlayPackets.h"

namespace Java {
	namespace Packet {
		namespace Server {
			namespace Play {
				static void confirmTeleportation(std::shared_ptr<::Client> client, const Java::Packet::Header& header, Watermelon* server) {
					uint32_t offset = 0;
					int32_t teleportId = Java::Packet::readVarInt(client, offset);
					Java::Packet::readDouble(client, offset); // x
					Java::Packet::readDouble(client, offset); // y
					Java::Packet::readDouble(client, offset); // z
					Java::Packet::readFloat(client, offset); // yaw
					Java::Packet::readFloat(client, offset); // pitch

					bool teleportOk = true;

					{
						std::lock_guard<std::mutex> lock(server->getClientMutex());
						teleportOk = client->teleportIdCounter == teleportId;
						client->teleportIdCounter++;
						client->teleporting = false;
					}

					if (!teleportOk) {
						server->addPacketToQueue(client, [client, server]() {
							Java::Packet::Client::Play::synchronizePlayerPosition(client, server);
						});
					}
				}

				void parse(std::shared_ptr<::Client> client, const Java::Packet::Header& header, Watermelon* server) {
					switch (header.packetId) {
					case (int32_t)Java::Packet::PacketId::StatusServer::STATUS_REQUEST:
						return confirmTeleportation(client, header, server);
					}
				}
			};
		};
	};
};
