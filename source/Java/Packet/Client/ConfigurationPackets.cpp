#include "Java/Packet/Packet.h"
#include "Java/Packet/Client/StatusPackets.h"

namespace Java {
	namespace Packet {
		namespace Client {
			namespace Configuration {
				void knownPacks(std::shared_ptr<::Client> client, Watermelon* server) {
					std::vector<char> buffer;

					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::ConfigurationClient::KNOWN_PACKS, buffer);
					Java::Packet::writePacks(server->getPacks(), buffer);
					server->sendDataToClient(client, buffer);
				}

				void registryData(std::shared_ptr<::Client> client, Watermelon* server) {
					std::vector<char> buffer;
					server->sendDataToClient(client, buffer);
				}
			};
		};
	};
};
