#include "Java/Packet/Client/PlayPackets.h"
#include "Java/Packet/Client/ConfigurationPackets.h"
#include "Java/Packet/Server/ConfigurationPackets.h"

namespace Java {
	namespace Packet {
		namespace Server {
			namespace Configuration {
				static void knownPacks(std::shared_ptr<::Client> client, int32_t packetId, Watermelon* server, const std::vector<uint8_t>& packet) {
					uint32_t offset = 0;

					try {
						std::vector<Java::Pack> packs = Java::Packet::readPacks(packet, offset);
						Java::Packet::checkEmpty(packet, offset);

						server->addOperationToQueue(client, [client, server, packs = std::move(packs)]() {
							client->setPacks(packs);
							Java::Packet::Client::Configuration::registryData(client, server);
							Java::Packet::Client::Configuration::updateTags(client, server);
							Java::Packet::Client::Configuration::finishConfiguration(client, server);
						});
					} catch(const std::runtime_error& e) {
						server->getLogger().log("Can't decode packet ID " + std::to_string(packetId) + " : " + e.what(), "Network", Logger::WARN);
						server->disconnectClient(client.get());
					}
				}

				static void AcknowledgeFinishConfiguration(std::shared_ptr<::Client> client, int32_t packetId, Watermelon* server, const std::vector<uint8_t>& packet) {
					uint32_t offset = 0;

					client->changeStatus(::Client::Status::PLAY);
					server->addOperationToQueue(client, [client, server]() {
						Java::Packet::Client::Play::login(client, server);
						Java::Packet::Client::Play::synchronizePlayerPosition(client, server);
					});
				}

				void parse(std::shared_ptr<::Client> client, int32_t packetId, Watermelon* server, const std::vector<uint8_t>& packet) {
					switch (packetId) {
						case (int)Java::Packet::PacketId::ConfigurationServer::KNOWN_PACKS:
							return knownPacks(client, packetId, server, packet);
						case (int)Java::Packet::PacketId::ConfigurationServer::ACKNOWLEDGE_FINISH_CONFIGURATION:
							return AcknowledgeFinishConfiguration(client, packetId, server, packet);
					}
				}
			};
		};
	};
};
