#include "Java/Packet/Client/PlayPackets.h"
#include "Java/Packet/Client/ConfigurationPackets.h"
#include "Java/Packet/Server/ConfigurationPackets.h"

namespace Java {
	namespace Packet {
		namespace Server {
			namespace Configuration {
				static void knownPacks(std::shared_ptr<::Client> client, const Java::Packet::Header& header, Watermelon* server) {
					uint32_t offset = 0;
					std::vector<Java::Pack> packs = Java::Packet::readPacks(client, offset);

					{
						std::lock_guard<std::mutex> lock(server->getClientMutex());
						client->packs = packs;
					}

					server->addPacketToQueue(client, [client, server]() {
						Java::Packet::Client::Configuration::registryData(client, server);
						Java::Packet::Client::Configuration::updateTags(client, server);
						Java::Packet::Client::Configuration::finishConfiguration(client, server);
					});
				}

				static void AcknowledgeFinishConfiguration(std::shared_ptr<::Client> client, const Java::Packet::Header& header, Watermelon* server) {
					uint32_t offset = 0;

					{
						std::lock_guard<std::mutex> lock(server->getClientMutex());
						client->status = ::Client::Status::PLAY;
					}

					server->addPacketToQueue(client, [client, server]() {
						Java::Packet::Client::Play::login(client, server);
						Java::Packet::Client::Play::synchronizePlayerPosition(client, server);
					});
				}

				void parse(std::shared_ptr<::Client> client, const Java::Packet::Header& header, Watermelon* server) {
					switch (header.packetId) {
						case (int)Java::Packet::PacketId::ConfigurationServer::KNOWN_PACKS:
							return knownPacks(client, header, server);
						case (int)Java::Packet::PacketId::ConfigurationServer::ACKNOWLEDGE_FINISH_CONFIGURATION:
							return AcknowledgeFinishConfiguration(client, header, server);
					}
				}
			};
		};
	};
};
