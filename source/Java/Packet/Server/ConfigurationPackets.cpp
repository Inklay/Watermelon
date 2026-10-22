#include "Java/Packet/Client/PlayPackets.h"
#include "Java/Packet/Client/ConfigurationPackets.h"
#include "Java/Packet/Server/ConfigurationPackets.h"

namespace Java {
	namespace Packet {
		namespace Server {
			namespace Configuration {
				static void knownPacks(const std::shared_ptr<::Client>& client, Watermelon* server, const std::vector<uint8_t>& packet) {
					uint32_t offset = 0;
					std::vector<Java::Pack> packs = Java::Packet::readPacks(packet, offset);
					Java::Packet::checkEmpty(packet, offset);

					server->addOperationToQueue(client, [client, packs = std::move(packs)]() {
						client->setPacks(packs);
						Java::Packet::Client::Configuration::registryData(client);
						Java::Packet::Client::Configuration::updateTags(client);
						Java::Packet::Client::Configuration::finishConfiguration(client);
					});
				}

				static void AcknowledgeFinishConfiguration(const std::shared_ptr<::Client>& client,  Watermelon* server, const std::vector<uint8_t>& packet) {
					uint32_t offset = 0;
					Java::Packet::checkEmpty(packet, offset);

					client->changeStatus(::Client::Status::PLAY);
					server->addOperationToQueue(client, [client, server]() {
						std::vector<Player::InfoAction> actions = { Player::InfoAction::ADD_PLAYER, Player::InfoAction::UPDATE_LISTED, Player::InfoAction::UPDATE_GAME_MODE, Player::InfoAction::UPDATE_LATENCY };
						Player::Player* player = client->getPlayer();

						Java::Packet::Client::Play::login(client, server);
						Java::Packet::Client::Play::synchronizePlayerPosition(client);
						Java::Packet::Client::Play::playerInfoUpdate(server, actions, { player });
						Java::Packet::Client::Play::gameEvent(client, Java::Packet::GameEvent::START_WAITING_FOR_CHUNKS);
						server->getLogger().log(player->getUsername() + " joined the game", "Watermelon", Logger::INFO);
						Java::Packet::Client::Play::setCenterChunk(client);
						Java::Packet::Client::Play::chunkDataAndUpdateLight(client, 0, 0);
					});
				}

				static void pluginMessage(const std::shared_ptr<::Client>& client, Watermelon* server, const std::vector<uint8_t>& packet) {
					uint32_t offset = 0;
					const std::string identifier = Java::Packet::readString(packet, offset);
					
					if (identifier == "minecraft:brand") {
						Java::Packet::readString(packet, offset); // Client's brand
						Java::Packet::checkEmpty(packet, offset);

						server->addOperationToQueue(client, [client, identifier]() {
							Java::Packet::Client::Configuration::pluginMessage(client, identifier, "Watermelon");
						});

						return;
					}

					throw parseError("Unknown channel");
				}

				static void clientInformation(const std::shared_ptr<::Client>& client, Watermelon* server, const std::vector<uint8_t>& packet) {
					uint32_t offset = 0;

					Java::Packet::readString(packet, offset); // Locale, unused
					uint8_t viewDistance = Java::Packet::readU8(packet, offset);
					uint32_t tempChatMode = Java::Packet::readVarInt(packet, offset);

					if (tempChatMode > 2) {
						throw parseError("Invalid chat mode");
					}

					Player::ChatMode chatMode = static_cast<Player::ChatMode>(tempChatMode);
					Java::Packet::readBool(packet, offset); // Chat color, unused
					uint8_t skinPart = Java::Packet::readU8(packet, offset);
					uint32_t mainHand = Java::Packet::readVarInt(packet, offset);
					Java::Packet::readBool(packet, offset); // Text filertering, unused
					bool shownOnServerList = Java::Packet::readBool(packet, offset);
					uint32_t particles = Java::Packet::readVarInt(packet, offset);
					Java::Packet::checkEmpty(packet, offset);

					Player::ClientInfo clientInfo = { viewDistance, chatMode, skinPart, mainHand, shownOnServerList, particles };

					server->addOperationToQueue(client, [client, server, clientInfo = std::move(clientInfo)]() {
						client->getPlayer()->setClientInfo(clientInfo);
						server->updatePlayerSample();
					});
				}

				void parse(const std::shared_ptr<::Client>& client, int32_t packetId, Watermelon* server, const std::vector<uint8_t>& packet) {
					switch (packetId) {
						case (int)Java::Packet::PacketId::ConfigurationServer::KNOWN_PACKS:
							return knownPacks(client, server, packet);
						case (int)Java::Packet::PacketId::ConfigurationServer::ACKNOWLEDGE_FINISH_CONFIGURATION:
							return AcknowledgeFinishConfiguration(client, server, packet);
						case (int)Java::Packet::PacketId::ConfigurationServer::CLIENT_INFORMATION:
							return clientInformation(client, server, packet);
						case (int)Java::Packet::PacketId::ConfigurationServer::PLUGIN_MESSAGE:
							return pluginMessage(client, server, packet);
						default:
							server->getLogger().log("Unknown packet ID: " + std::to_string(packetId), "Network", Logger::ERR);
							server->disconnectClient(client.get(), std::string("disconnect.packetError"));
					}
				}
			};
		};
	};
};
