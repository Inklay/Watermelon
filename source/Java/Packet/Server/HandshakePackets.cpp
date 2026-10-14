#include "Java/Packet/Server/HandshakePackets.h"

namespace Java {
	namespace Packet {
		namespace Server {
			namespace Handshake {
				static void handshake(std::shared_ptr<::Client> client, const Java::Packet::Header& header, Watermelon* server) {
					if (header.length == 0) {
						return;
					}

					uint32_t offset = 0;
					int32_t protocolVersion = Java::Packet::readVarInt(client, offset);
					std::string serverAddress = Java::Packet::readString(client, offset);
					uint16_t serverPort = Java::Packet::readU16(client, offset);
					Java::Packet::Intent intent = static_cast<Java::Packet::Intent>(Java::Packet::readVarInt(client, offset));

					{
						std::lock_guard<std::mutex> lock(server->getClientMutex());
						switch (intent) {
						case Java::Packet::Intent::STATUS:
							client->status = ::Client::Status::STATUS;
							break;
						case Java::Packet::Intent::LOGIN:
						case Java::Packet::Intent::TRANSFER:
							client->status = ::Client::Status::LOGIN;
							break;
						}
					}
				}

				void parse(std::shared_ptr<::Client> client, const Java::Packet::Header& header, Watermelon* server) {
					switch (header.packetId) {
					case (int32_t)Java::Packet::PacketId::HandshakeServer::HANDSHAKE:
						return handshake(client, header, server);
					}
				}
			};
		};
	};
};
