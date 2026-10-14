#include "Java/Parser/ConfigurationPackets.h"

namespace Java {
	namespace Parser {
		namespace ConfigurationPackets {
			static void knownPacks(std::shared_ptr<Client> client, const Java::Packet::Header& header, Watermelon* server) {
				uint32_t offset = 0;
				uint32_t packsCount = Java::Packet::readVarInt(client, offset);
				std::string nameSpace = Java::Packet::readString(client, offset);
				std::string id = Java::Packet::readString(client, offset);
				std::string version = Java::Packet::readString(client, offset);

				std::cout << "Client " << client->player.username << " knows " << packsCount << " packs: " << nameSpace << ":" << id << " v" << version << std::endl;
			}

			void parse(std::shared_ptr<Client> client, const Java::Packet::Header& header, Watermelon* server) {
				switch (header.packetId) {
					case (int)Java::Packet::PacketId::ConfigurationServer::KNOWN_PACKS:
						return knownPacks(client, header, server);
				}
			}
		};
	};
};
