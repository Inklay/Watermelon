#include "Java/Parser/LoginPackets.h"
#include "Java/Handler/LoginPackets.h"

namespace Java {
	namespace Parser {
		namespace LoginPackets {
			static void loginStart(std::shared_ptr<Client> client, const Java::Packet::Header& header, Watermelon* server) {
				uint32_t offset = 0;
				std::string username = Java::Packet::readString(client, offset);
				std::pair<uint64_t, uint64_t> uuid = Java::Packet::readUUID(client, offset);

				server->addPacketToQueue(client, [client, server, username, uuid]() {
					return Java::Handler::Login::loginStartPacket(client, server, username, uuid);
				});
			}

			static void loginAknowledged(std::shared_ptr<Client> client, const Java::Packet::Header& header, Watermelon* server) {
				uint32_t offset = 0;
				std::string username = Java::Packet::readString(client, offset);
				std::pair<uint64_t, uint64_t> uuid = Java::Packet::readUUID(client, offset);

				{
					std::lock_guard<std::mutex> lock(server->getClientMutex());
					client->status = Client::Status::CONFIGURATION;
				}
			}

			void parse(std::shared_ptr<Client> client, const Java::Packet::Header& header, Watermelon* server) {
				switch (header.packetId) {
					case Java::Packet::PacketId::LoginServer::LOGIN_START:
						return loginStart(client, header, server);
					case Java::Packet::PacketId::LoginServer::LOGIN_AKNOWLEDGED:
						return loginAknowledged(client, header, server);
				}
			}
		};
	};
};
