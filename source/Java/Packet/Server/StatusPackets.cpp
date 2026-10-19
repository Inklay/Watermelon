#include "Java/Packet/Server/StatusPackets.h"
#include "Java/Packet/Client/StatusPackets.h"

namespace Java {
	namespace Packet {
		namespace Server {
			namespace Status {
				static void status(std::shared_ptr<::Client> client, Watermelon* server, const std::vector<uint8_t>& packet) {
					uint32_t offset = 0;
					Java::Packet::checkEmpty(packet, offset);

					server->addOperationToQueue(client, [client, server]() {
						return Java::Packet::Client::Status::statusPacket(client, server);
					});
				}

				static void ping(std::shared_ptr<::Client> client, const std::vector<uint8_t>& packet) {
					uint32_t offset = 0;
					int64_t ping = Java::Packet::readU64(packet, offset);
					Java::Packet::checkEmpty(packet, offset);

					std::vector<uint8_t> buffer;
					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::StatusClient::PONG, buffer);
					Java::Packet::write64(ping, buffer);
					client->sendData(buffer);
				}

				void parse(std::shared_ptr<::Client> client, int32_t packetId, Watermelon* server, const std::vector<uint8_t>& packet) {
					switch (packetId) {
					case (int32_t)Java::Packet::PacketId::StatusServer::STATUS_REQUEST:
						return status(client, server, packet);
					case (int32_t)Java::Packet::PacketId::StatusServer::PING:
						return ping(client, packet);
					default:
						server->getLogger().log("Unknown packet ID: " + std::to_string(packetId), "Network", Logger::ERR);
						server->disconnectClient(client.get());
					}
				}
			};
		};
	};
};
