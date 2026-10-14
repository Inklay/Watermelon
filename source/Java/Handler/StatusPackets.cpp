#include "Java/Handler/StatusPackets.h"
#include "Utils/JSON.h"
#include "Java/Packet.h"

using json = nlohmann::json;

namespace Java {
	namespace Handler {
		namespace Status {
			std::vector<char> statusPacket(std::shared_ptr<Client> client, Watermelon* server) {
				std::vector<char> buffer;
				json data = {
					{"version", {
						{"name", server->getVersion().name},
						{"protocol", server->getVersion().protocol}
					}},
					{"players", {
						{"max", server->getConfig()["networking"]["java"]["max_players"].value_or(10)},
						{"online", 0},
						{"sample", json::array()}
					}},
					{"description", {
						{"text", server->getConfig()["networking"]["java"]["motd"].value_or("A (hopefully) blazingly fast Watermelon server!")}
					}}
				};

				Java::Packet::writeVarInt(Java::Packet::PacketId::StatusClient::STATUS_RESPONSE, buffer);
				Java::Packet::writeString(data.dump(), buffer);
				return buffer;
			}
		};
	};
};
