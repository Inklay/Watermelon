#include "Java/Packet/Server/LoginPackets.h"
#include "Java/Packet/Client/LoginPackets.h"
#include "Java/Packet/Client/ConfigurationPackets.h"
#include <nlohmann/json.hpp>
#include "Utils/UUID.h"

namespace Java {
	namespace Packet {
		namespace Server {
			namespace Login {
				static void encryptionResponse(const std::shared_ptr<::Client>& client, Watermelon* server, const std::vector<uint8_t>& packet) {
					uint32_t offset = 0;
					const std::vector<uint8_t> encryptedSecret = Java::Packet::readByteArray(packet, offset);
					const std::vector<uint8_t> encryptedToken = Java::Packet::readByteArray(packet, offset);
					const std::vector<uint8_t> token = server->getKeys().decryptData(encryptedToken);
					const std::vector<uint8_t> secret = server->getKeys().decryptData(encryptedSecret);
					Java::Packet::checkEmpty(packet, offset);

					server->addOperationToQueue(client, [client, server, token = std::move(token), secret = std::move(secret)]() {
						if (!client->authenticate(token, secret)) {
							server->disconnectClient(client.get(), std::string("disconnect.loginFailedInfo.invalidSession"));
							return;
						}
						
						if (server->isOnlineMode()) {
							server->addAuthTask(client);
						} else {
							std::pair<uint64_t, uint64_t> uuid = Utils::UUID::offlineUUID(client->getTempUsername());
							client->createPlayer(uuid, server->increaseEntityCounter(), { 0, 0, 0 }, { 0, 0 });
							client->startEncryption();
							Java::Packet::Client::Login::setCompression(client);
							Java::Packet::Client::Login::loginSuccess(client);
						}
					});
				}

				static void loginStart(const std::shared_ptr<::Client>& client, Watermelon* server, const std::vector<uint8_t>& packet) {
					if (client->hasPlayer()) {
						server->getLogger().log("Player was already logged in", "Network", Logger::WARN);
						server->disconnectClient(client.get(), std::string("multiplayer.disconnect.duplicate_login"));
						return;
					}

					uint32_t offset = 0;
					std::string username = Java::Packet::readString(packet, offset);
					Java::Packet::readUUID(packet, offset); // UUID
					Java::Packet::checkEmpty(packet, offset);

					server->addOperationToQueue(client, [client, server, username = std::move(username)]() {
						if (!client->startLogin(username)) {
							return;
						}

						if (server->isEncryptionEnabled()) {
							Java::Packet::Client::Login::encryptionRequest(client, server);
						} else {
							std::pair<uint64_t, uint64_t> uuid = Utils::UUID::offlineUUID(username);
							client->createPlayer(uuid, server->increaseEntityCounter(), { 0, 0, 0 }, { 0, 0 });
							Java::Packet::Client::Login::setCompression(client);
							Java::Packet::Client::Login::loginSuccess(client);
						}
					});
				}

				static void loginAknowledged(const std::shared_ptr<::Client>& client, Watermelon* server, const std::vector<uint8_t>& packet) {
					if (!client->hasPlayer()) {
						server->getLogger().log("Player didn't log in", "Network", Logger::WARN);
						server->disconnectClient(client.get(), std::string("disconnect.packetError"));
						return;
					}
					
					uint32_t offset = 0;
					Java::Packet::checkEmpty(packet, offset);

					client->changeStatus(::Client::Status::CONFIGURATION);
					server->addOperationToQueue(client, [client, server]() {
						Java::Packet::Client::Configuration::knownPacks(client, server);
					});
				}

				static void loginPluginResponse(const std::vector<uint8_t>& packet) {
					uint32_t offset = 0;
					Java::Packet::readByteArray(packet, offset); // Data
					Java::Packet::checkEmpty(packet, offset);
				}

				static void cookieResponse(const std::vector<uint8_t>& packet) {
					uint32_t offset = 0;
					Java::Packet::readString(packet, offset); // Identifier
					Java::Packet::readByteArray(packet, offset); // Payload
					Java::Packet::checkEmpty(packet, offset);
				}

				void parse(const std::shared_ptr<::Client>& client, int32_t packetId, Watermelon* server, const std::vector<uint8_t>& packet) {
					switch (packetId) {
					case (int32_t)Java::Packet::PacketId::LoginServer::LOGIN_START:
						return loginStart(client, server, packet);
					case (int32_t)Java::Packet::PacketId::LoginServer::ENCRYPTION_RESPONSE:
						return encryptionResponse(client, server, packet);
					case (int32_t)Java::Packet::PacketId::LoginServer::LOGIN_ACKNOWLEDGED:
						return loginAknowledged(client, server, packet);
					case (int32_t)Java::Packet::PacketId::LoginServer::LOGIN_PLUGIN_RESPONSE:
						return loginPluginResponse(packet);
					case (int32_t)Java::Packet::PacketId::LoginServer::COOKIE_RESPONSE:
						return cookieResponse(packet);
					default:
						server->getLogger().log("Unknown packet ID: " + std::to_string(packetId), "Network", Logger::ERR);
						server->disconnectClient(client.get(), std::string("disconnect.packetError"));
					}
				}
			};
		};
	};
};
