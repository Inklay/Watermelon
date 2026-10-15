#include "Java/Packet/Server/StatusPackets.h"
#include "Java/Packet/Client/StatusPackets.h"

namespace Java {
	namespace Packet {
		namespace Server {
			namespace Status {
				static void status(std::shared_ptr<::Client> client, int32_t packetId, Watermelon* server, const std::vector<uint8_t>& packet) {
					server->addOperationToQueue(client, [client, server]() {
						return Java::Packet::Client::Status::statusPacket(client, server);
					});
				}

				static void ping(std::shared_ptr<::Client> client, int32_t packetId, Watermelon* server, const std::vector<uint8_t>& packet) {
					uint32_t offset = 0;
					try {
						int64_t ping = Java::Packet::readU64(packet, offset);
						Java::Packet::checkEmpty(packet, offset);

						std::vector<char> buffer;
						Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::StatusClient::PONG, buffer);
						Java::Packet::write64(ping, buffer);
						server->sendDataToClient(client, buffer);
					} catch (const std::runtime_error& e) {
						server->getLogger().log("Can't decode packet ID " + std::to_string(packetId) + " : " + e.what(), "Network", Logger::WARN);
						server->disconnectClient(client.get());
					}
				}

				void parse(std::shared_ptr<::Client> client, int32_t packetId, Watermelon* server, const std::vector<uint8_t>& packet) {
					switch (packetId) {
					case (int32_t)Java::Packet::PacketId::StatusServer::STATUS_REQUEST:
						return status(client, packetId, server, packet);
					case (int32_t)Java::Packet::PacketId::StatusServer::PING:
						return ping(client, packetId, server, packet);
					default:
						server->getLogger().log("Unknown packet ID: " + std::to_string(packetId), "Network", Logger::WARN);
						server->disconnectClient(client.get());
					}
				}
			};
		};
	};
};
