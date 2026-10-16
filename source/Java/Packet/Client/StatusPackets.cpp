#include "Utils/JSON.h"
#include "Java/Packet/Packet.h"
#include "Java/Packet/Client/StatusPackets.h"

using json = nlohmann::json;

namespace Java {
	namespace Packet {
		namespace Client {
			namespace Status {
				void statusPacket(std::shared_ptr<::Client> client, Watermelon* server) {
					std::vector<char> buffer;
					json data = {
						{"version", {
							{"name", server->getVersion().name},
							{"protocol", server->getVersion().protocol}
						}},
						{"players", {
							{"max", server->getConfig()["networking"]["java"]["max_players"].value_or(10)},
							{"online", server->getPlayerCount()},
							{"sample", json::array()}
						}},
						{"description", {
							{"text", server->getConfig()["networking"]["java"]["motd"].value_or("A (hopefully) blazingly fast Watermelon server!")}
						}}
					};

					const std::string faviconData = server->getFaviconData();
					if (!faviconData.empty()) {
						data["favicon"] = faviconData;
					}

					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::StatusClient::STATUS_RESPONSE, buffer);
					Java::Packet::writeString(data.dump(), buffer);
					server->sendDataToClient(client, buffer);
				}
			};
		};
	};
};
