#include "Java/Packet/Packet.h"
#include "Java/Packet/Client/StatusPackets.h"
#include "Registry/RegistryManager.h"

namespace Java {
	namespace Packet {
		namespace Client {
			namespace Configuration {
				void knownPacks(std::shared_ptr<::Client> client, Watermelon* server) {
					std::vector<uint8_t> buffer;

					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::ConfigurationClient::KNOWN_PACKS, buffer);
					Java::Packet::writePacks(server->getPacks(), buffer);
					server->sendDataToClient(client, buffer);
				}

				void finishConfiguration(std::shared_ptr<::Client> client, Watermelon* server) {
					std::vector<uint8_t> buffer;

					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::ConfigurationClient::FINISH_CONFIGURATION, buffer);
					server->sendDataToClient(client, buffer);
				}

				void updateTags(std::shared_ptr<::Client> client, Watermelon* server) {
					std::vector<uint8_t> buffer;
					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::ConfigurationClient::UPDATE_TAGS, buffer);
					Java::Packet::writeTags(RegistryManager::getInstance().getRegistryTags(), buffer);
					server->sendDataToClient(client, buffer);
				}

				void registryData(std::shared_ptr<::Client> client, Watermelon* server) {
					for (const Registry* syncReg : RegistryManager::getInstance().getSynchronizedRegistries()) {
						std::vector<uint8_t> buffer;

						Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::ConfigurationClient::REGISTRY_DATA, buffer);

						if (syncReg->name.starts_with("minecraft:") || client->hasPack(syncReg->name)) {
							Java::Packet::writeRegistry(syncReg, true, buffer);
						} else {
							Java::Packet::writeRegistry(syncReg, false, buffer);
						}

						server->sendDataToClient(client, buffer);
					}
				}

				void pluginMessage(std::shared_ptr<::Client> client, Watermelon* server, const std::string& channel, const std::string& data) {
					std::vector<uint8_t> buffer;
					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::ConfigurationClient::PLUGIN_MESSAGE, buffer);
					Java::Packet::writeString(channel, buffer);
					Java::Packet::writeString(data, buffer);
					server->sendDataToClient(client, buffer);
				}
			};
		};
	};
};
