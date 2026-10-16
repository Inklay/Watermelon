#include <string>
#include "Java/Packet/Server/Handler.h"
#include "Java/Packet/Server/PlayPackets.h"
#include "Java/Packet/Server/LoginPackets.h"
#include "Java/Packet/Server/StatusPackets.h"
#include "Java/Packet/Server/HandshakePackets.h"
#include "Java/Packet/Server/ConfigurationPackets.h"

namespace Java {
	namespace Packet {
		namespace Server {
			namespace Handler {
				void handlePacket(std::shared_ptr<::Client> client,  Watermelon* server, std::vector<uint8_t>& packet) {
					int32_t packetId = 0;
					uint32_t offset = 0;

					try {
						packetId = Java::Packet::readVarInt(packet, offset);
					} catch (const std::runtime_error& e) {
						server->getLogger().log(std::string("Can't get packet id : ") + e.what(), "Network", Logger::WARN);
						server->disconnectClient(client.get());
						return;
					}

					packet.erase(packet.begin(), packet.begin() + offset);

					if (client->getStatus() != ::Client::Status::PLAY || packetId != (int32_t)Java::Packet::PacketId::PlayServer::CLIENT_TICK_END) {
						server->getLogger().log("Received java packet with ID: " + std::to_string(packetId) + " and client status: " + std::to_string(client->getStatus()), "Network", Logger::DEBUG);
					}

					try {
						switch (client->getStatus()) {
						case Client::Status::HANDSHAKING:
							return Java::Packet::Server::Handshake::parse(client, packetId, server, packet);
						case Client::Status::STATUS:
							return Java::Packet::Server::Status::parse(client, packetId, server, packet);
						case Client::Status::LOGIN:
							return Java::Packet::Server::Login::parse(client, packetId, server, packet);
						case Client::Status::CONFIGURATION:
							return Java::Packet::Server::Configuration::parse(client, packetId, server, packet);
						case Client::Status::PLAY:
							return Java::Packet::Server::Play::parse(client, packetId, server, packet);
						}
					} catch (const Java::Packet::parseError& e) {
						server->getLogger().log("Can't decode packet ID " + std::to_string(packetId) + " : " + e.what(), "Network", Logger::WARN);
						server->disconnectClient(client.get());
						return;
					}
				}
			};
		};
	};
};
