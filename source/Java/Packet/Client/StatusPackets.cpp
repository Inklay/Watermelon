#include <nlohmann/json.hpp>
#include "Java/Packet/Packet.h"
#include "Java/Packet/Client/StatusPackets.h"
#include "Utils/UUID.h"

namespace Java {
	namespace Packet {
		namespace Client {
			namespace Status {
				void statusPacket(const std::shared_ptr<::Client>& client, Watermelon* server) {
					std::vector<uint8_t> buffer;
					nlohmann::json playerSample = nlohmann::json::array();

					for (const auto& player : server->getPlayerSample()) {
						playerSample.push_back({
							{ "name", player.first },
							{ "id", Utils::UUID::formatUUID(player.second) }
						});
					}

					nlohmann::json data = {
						{ "version", {
							{ "name", server->getVersion().name },
							{ "protocol", server->getVersion().protocol }
						}},
						{ "players", {
							{ "max", server->getConfig()["networking"]["java"]["max_players"].value_or(10) },
							{ "online", server->getPlayerCount() },
							{ "sample", playerSample }
						}},
						{ "description", {
							{ "text", server->getConfig()["networking"]["java"]["motd"].value_or("A (hopefully) blazingly fast Watermelon server!") }
						}}
					};

					const std::string faviconData = server->getFaviconData();
					if (!faviconData.empty()) {
						data["favicon"] = faviconData;
					}

					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::StatusClient::STATUS_RESPONSE, buffer);
					Java::Packet::writeString(data.dump(), buffer);
					client->sendData(buffer);
				}
			};
		};
	};
};
