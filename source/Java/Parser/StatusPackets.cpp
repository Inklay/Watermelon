#include "Java/Parser/StatusPackets.h"
#include "Java/Handler/StatusPackets.h"

namespace Java {
	namespace Parser {
		namespace StatusPackets {
			static void status(std::shared_ptr<Client> client, const Java::Packet::Header& header, Watermelon* server) {
				server->addPacketToQueue(client, [client, server]() {
					return Java::Handler::Status::statusPacket(client, server);
				});
			}

			static void ping(std::shared_ptr<Client> client, const Java::Packet::Header& header, Watermelon* server) {
				uint32_t offset = 0;
				int64_t ping = Java::Packet::read64(client, offset);

				std::vector<char> buffer;
				Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::StatusClient::PONG, buffer);
				Java::Packet::write64(ping, buffer);
				server->sendDataToClient(client, buffer);
			}

			void parse(std::shared_ptr<Client> client, const Java::Packet::Header& header, Watermelon* server) {
				switch (header.packetId) {
					case (int32_t)Java::Packet::PacketId::StatusServer::STATUS_REQUEST:
						return status(client, header, server);
					case (int32_t)Java::Packet::PacketId::StatusServer::PING:
						return ping(client, header, server);
				}
			}
		};
	};
};
