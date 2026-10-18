#include "Java/Packet/Server/LoginPackets.h"
#include "Java/Packet/Client/LoginPackets.h"
#include "Java/Packet/Client/ConfigurationPackets.h"
#include "Utils/JSON.h"
#include "Utils/UUID.h"

using json = nlohmann::json;

namespace Java {
	namespace Packet {
		namespace Server {
			namespace Login {
				static void encryptionResponse(std::shared_ptr<::Client> client, Watermelon* server, const std::vector<uint8_t>& packet) {
					uint32_t offset = 0;
					const std::vector<uint8_t> encryptedSecret = Java::Packet::readByteArray(packet, offset);
					const std::vector<uint8_t> encryptedToken = Java::Packet::readByteArray(packet, offset);
					Java::Packet::checkEmpty(packet, offset);

					server->addOperationToQueue(client, [client, server, encryptedSecret = std::move(encryptedSecret), encryptedToken = std::move(encryptedToken)]() {
						Player* player = client->getPlayer();

						std::vector<uint8_t> token = server->getKeys().decryptData(encryptedToken);

						if (!client->checkToken(token)) {
							server->getLogger().log("Unable to verify " + client->getPlayer()->getUsername() + "'s encryption tokens", "Network", Logger::WARN);
							server->disconnectClient(client.get());
							return;
						}

						std::vector<uint8_t> secret = server->getKeys().decryptData(encryptedSecret);

						if (secret.size() != 16) {
							server->getLogger().log("Unable to verify " + client->getPlayer()->getUsername() + "'s encryption secret", "Network", Logger::WARN);
							server->disconnectClient(client.get());
							return;
						}

						client->startEncryption(secret);
						Java::Packet::Client::Login::loginSuccess(client, server, player->getUsername(), player->getUUID());
					});
				}

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
						if (!client->createPlayer(username, uuid, server->increaseEntityCounter(), { 0, 0, 0 }, { 0, 0 })) {
							json reason = {
								{ "translate", "multiplayer.disconnect.unverified_username" }
							};

							return Java::Packet::Client::Login::disconnect(client, server, reason.dump());
						}
						server->getLogger().log("UUID of player " + username + " is " + Utils::UUID::formatUUID(uuid), "Network", Logger::INFO);

						if (server->getConfig()["networking"]["java"]["encryption"].value_or(true)) {
							Java::Packet::Client::Login::encryptionRequest(client, server);
						} else {
							Java::Packet::Client::Login::loginSuccess(client, server, username, uuid);
						}
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
					case (int32_t)Java::Packet::PacketId::LoginServer::ENCRYPTION_RESPONSE:
						return encryptionResponse(client, server, packet);
					case (int32_t)Java::Packet::PacketId::LoginServer::LOGIN_ACKNOWLEDGED:
						return loginAknowledged(client, server, packet);
					default:
						server->getLogger().log("Unknown packet ID: " + std::to_string(packetId), "Network", Logger::ERR);
						server->disconnectClient(client.get());
					}
				}
			};
		};
	};
};
