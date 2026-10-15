#include "Java/Packet/Server/HandshakePackets.h"

namespace Java {
	namespace Packet {
		namespace Server {
			namespace Handshake {
				static void handshake(std::shared_ptr<::Client> client, int32_t packetId, Watermelon* server, const std::vector<uint8_t>& packet) {
					uint32_t offset = 0;
					
					try {
						int32_t protocolVersion = Java::Packet::readVarInt(packet, offset);

						if (protocolVersion != server->getVersion().protocol) {
							server->getLogger().log("Client tried to connect with wrong protocol version " + std::to_string(protocolVersion), "Network", Logger::WARN);
							server->disconnectClient(client.get());
							return;
						}

						std::string serverAddress = Java::Packet::readString(packet, offset);
						uint16_t serverPort = Java::Packet::readU16(packet, offset);
						Java::Packet::Intent intent = static_cast<Java::Packet::Intent>(Java::Packet::readVarInt(packet, offset));
						Java::Packet::checkEmpty(packet, offset);

						switch (intent) {
						case Java::Packet::Intent::STATUS:
							client->changeStatus(::Client::Status::STATUS);
							break;
						case Java::Packet::Intent::LOGIN:
						case Java::Packet::Intent::TRANSFER:
							client->changeStatus(::Client::Status::LOGIN);
							break;
						default:
							throw std::runtime_error("Invalid intent");
						}
					} catch (const std::runtime_error& e) {
						server->getLogger().log("Can't decode packet ID " + std::to_string(packetId) + " : " + e.what(), "Network", Logger::WARN);
						server->disconnectClient(client.get());
						return;
					}
				}

				void parse(std::shared_ptr<::Client> client, int32_t packetId, Watermelon* server, const std::vector<uint8_t>& packet) {
					switch (packetId) {
					case (int32_t)Java::Packet::PacketId::HandshakeServer::HANDSHAKE:
						return handshake(client, packetId, server, packet);
					default:
						server->getLogger().log("Unknown packet ID: " + std::to_string(packetId), "Network", Logger::WARN);
						server->disconnectClient(client.get());
					}
				}
			};
		};
	};
};
