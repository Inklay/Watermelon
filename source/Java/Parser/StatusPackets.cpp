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
				Java::Packet::writeVarInt(client, Java::Packet::PacketId::Status::PING, buffer);
				Java::Packet::write64(client, ping, buffer);
				server->sendDataToClient(client, buffer);
			}

			void parse(std::shared_ptr<Client> client, const Java::Packet::Header& header, Watermelon* server) {
				switch (header.packetId) {
					case Java::Packet::PacketId::Status::STATUS:
						return status(client, header, server);
					case Java::Packet::PacketId::Status::PING:
						return ping(client, header, server);
				}
			}
		};
	};
};
