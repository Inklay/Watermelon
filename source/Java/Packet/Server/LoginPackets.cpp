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
					const std::vector<uint8_t> token = server->getKeys().decryptData(encryptedToken);
					const std::vector<uint8_t> secret = server->getKeys().decryptData(encryptedSecret);
					Java::Packet::checkEmpty(packet, offset);

					server->addOperationToQueue(client, [client, server, token = std::move(token), secret = std::move(secret)]() {
						if (!client->authenticate(token, secret)) {
							server->disconnectClient(client.get());
							return;
						}
						
						if (server->isOnlineMode()) {
							server->addAuthTask(client);
						} else {
							std::pair<uint64_t, uint64_t> uuid = Utils::UUID::offlineUUID(client->getTempUsername());
							client->createPlayer(uuid, server->increaseEntityCounter(), { 0, 0, 0 }, { 0, 0 });
							client->startEncryption();
							Player* player = client->getPlayer();
							Java::Packet::Client::Login::setCompression(client);
							Java::Packet::Client::Login::loginSuccess(client, player->getUsername(), player->getUUID());
						}
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
					Java::Packet::readUUID(packet, offset); // UUID
					Java::Packet::checkEmpty(packet, offset);

					server->addOperationToQueue(client, [client, server, username = std::move(username)]() {
						if (!client->startLogin(username)) {
							json reason = {
								{ "translate", "multiplayer.disconnect.unverified_username" }
							};

							return Java::Packet::Client::Login::disconnect(client, server, reason.dump());
						}
						if (server->isEncryptionEnabled()) {
							Java::Packet::Client::Login::encryptionRequest(client, server);
						} else {
							std::pair<uint64_t, uint64_t> uuid = Utils::UUID::offlineUUID(username);
							client->createPlayer(uuid, server->increaseEntityCounter(), { 0, 0, 0 }, { 0, 0 });
							Java::Packet::Client::Login::setCompression(client);
							Java::Packet::Client::Login::loginSuccess(client, username, uuid);
						}
					});
				}

				static void loginAknowledged(std::shared_ptr<::Client> client, Watermelon* server, const std::vector<uint8_t>& packet) {
					if (!client->hasPlayer()) {
						server->getLogger().log("Player didn't log in", "Network", Logger::WARN);
						server->disconnectClient(client.get());
						return;
					}
					
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
