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
						{"max", 100},
						{"online", 0},
						{"sample", json::array()}
					}},
					{"description", {
						{"text", "Watermelon Server"}
					}}
				};

				Java::Packet::writeVarInt(client, Java::Packet::PacketId::Status::STATUS, buffer);
				Java::Packet::writeString(client, data.dump(), buffer);
				return buffer;
			}
		};
	};
};
