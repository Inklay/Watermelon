#include "Java/Handler/StatusPackets.h"
#include "Java/Packet.h"

namespace Java {
	namespace Handler {
		namespace Configuration {
			std::vector<char> registryDataPacket(std::shared_ptr<Client> client, Watermelon* server) {
				std::vector<char> buffer;

				Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::ConfigurationClient::KNOWN_PACKS, buffer);
				Java::Packet::writeVarInt(1, buffer);
				Java::Packet::writeString("minecraft", buffer);
				Java::Packet::writeString("core", buffer);
				Java::Packet::writeString(server->getVersion().name, buffer);
				return buffer;
			}
		};
	};
};
