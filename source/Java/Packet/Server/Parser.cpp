#include <string>
#include "Java/Packet/Server/Parser.h"
#include "Java/Packet/Server/PlayPackets.h"
#include "Java/Packet/Server/LoginPackets.h"
#include "Java/Packet/Server/StatusPackets.h"
#include "Java/Packet/Server/HandshakePackets.h"
#include "Java/Packet/Server/ConfigurationPackets.h"

namespace Java {
	namespace Packet {
		namespace Server {
			namespace Parser {
				Java::Packet::Header parseHeader(std::shared_ptr<::Client> client) {
					uint32_t offset = 0;
					int32_t packetLength = Java::Packet::readVarInt(client, offset);
					int32_t packetId = Java::Packet::readVarInt(client, offset);

					if (packetId == -1 || packetLength == -1) {
						return Java::Packet::Header{ -1, -1 };
					}

					return Java::Packet::Header{ packetLength, packetId };
				}

				void parsePacket(std::shared_ptr<::Client> client, const Java::Packet::Header& header, Watermelon* server) {
					if (client->getStatus() != ::Client::Status::PLAY || header.packetId != (int32_t)Java::Packet::PacketId::PlayServer::CLIENT_TICK_END) {
						server->getLogger().log("Received packet with ID: " + std::to_string(header.packetId) + " and client status: " + std::to_string(client->getStatus()), "Java Parser", Logger::DEBUG);
					}
					switch (client->getStatus()) {
					case Client::Status::HANDSHAKING:
						return Java::Packet::Server::Handshake::parse(client, header, server);
					case Client::Status::STATUS:
						return Java::Packet::Server::Status::parse(client, header, server);
					case Client::Status::LOGIN:
						return Java::Packet::Server::Login::parse(client, header, server);
					case Client::Status::CONFIGURATION:
						return Java::Packet::Server::Configuration::parse(client, header, server);
					case Client::Status::PLAY:
						return Java::Packet::Server::Play::parse(client, header, server);
					}
				}
			};
		};
	};
};
