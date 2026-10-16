#include "Java/Packet/Server/LoginPackets.h"
#include "Java/Packet/Client/LoginPackets.h"
#include "Java/Packet/Client/ConfigurationPackets.h"

namespace Java {
	namespace Packet {
		namespace Server {
			namespace Login {
				static void loginStart(std::shared_ptr<::Client> client, Watermelon* server, const std::vector<uint8_t>& packet) {
					if (client->hasPlayer()) {
						server->getLogger().log("Player was already logged in", "Network", Logger::WARN);
						server->disconnectClient(client.get());
						return;
					}

					uint32_t offset = 0;
					std::string username = Java::Packet::readString(packet, offset);
					std::pair<uint64_t, uint64_t> uuid = Java::Packet::readUUID(packet, offset);
					Java::Packet::checkEmpty(packet, offset);

					server->addOperationToQueue(client, [client, server, username = std::move(username), uuid = std::move(uuid)]() {
						return Java::Packet::Client::Login::loginStart(client, server, username, uuid);
					});
				}

				static void loginAknowledged(std::shared_ptr<::Client> client, Watermelon* server, const std::vector<uint8_t>& packet) {
					client->getPlayer();
					
					uint32_t offset = 0;
					Java::Packet::checkEmpty(packet, offset);

					client->changeStatus(::Client::Status::CONFIGURATION);
					server->addOperationToQueue(client, [client, server]() {
						Java::Packet::Client::Configuration::knownPacks(client, server);
					});
				}

				void parse(std::shared_ptr<::Client> client, int32_t packetId, Watermelon* server, const std::vector<uint8_t>& packet) {
					switch (packetId) {
					case (int32_t)Java::Packet::PacketId::LoginServer::LOGIN_START:
						return loginStart(client, server, packet);
					case (int32_t)Java::Packet::PacketId::LoginServer::LOGIN_ACKNOWLEDGED:
						return loginAknowledged(client, server, packet);
					default:
						server->getLogger().log("Unknown packet ID: " + std::to_string(packetId), "Network", Logger::WARN);
						server->disconnectClient(client.get());
					}
				}
			};
		};
	};
};
