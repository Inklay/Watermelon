#include "Java/Packet/Server/LoginPackets.h"
#include "Java/Packet/Client/LoginPackets.h"
#include "Java/Packet/Client/ConfigurationPackets.h"

namespace Java {
	namespace Packet {
		namespace Server {
			namespace LoginPackets {
				static void loginStart(std::shared_ptr<::Client> client, const Java::Packet::Header& header, Watermelon* server) {
					uint32_t offset = 0;
					std::string username = Java::Packet::readString(client, offset);
					std::pair<uint64_t, uint64_t> uuid = Java::Packet::readUUID(client, offset);

					server->addPacketToQueue(client, [client, server, username, uuid]() {
						return Java::Packet::Client::Login::loginStartPacket(client, server, username, uuid);
						});
				}

				static void loginAknowledged(std::shared_ptr<::Client> client, const Java::Packet::Header& header, Watermelon* server) {
					uint32_t offset = 0;
					std::string username = Java::Packet::readString(client, offset);
					std::pair<uint64_t, uint64_t> uuid = Java::Packet::readUUID(client, offset);

					{
						std::lock_guard<std::mutex> lock(server->getClientMutex());
						client->status = ::Client::Status::CONFIGURATION;
					}

					server->addPacketToQueue(client, [client, server]() {
						return Java::Packet::Client::Configuration::knownPacks(client, server);
						});
				}

				void parse(std::shared_ptr<::Client> client, const Java::Packet::Header& header, Watermelon* server) {
					switch (header.packetId) {
					case (int32_t)Java::Packet::PacketId::LoginServer::LOGIN_START:
						return loginStart(client, header, server);
					case (int32_t)Java::Packet::PacketId::LoginServer::LOGIN_ACKNOWLEDGED:
						return loginAknowledged(client, header, server);
					}
				}
			};
		};
	};
};
