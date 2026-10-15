#include "Utils/UUID.h"
#include "Java/Packet/Packet.h"
#include "Java/Packet/Client/StatusPackets.h"
#include "Utils/JSON.h"

using json = nlohmann::json;

namespace Java {
	namespace Packet {
		namespace Client {
			namespace Login {
				void disconnect(std::shared_ptr<::Client> client, Watermelon* server, const std::string& reason) {
					std::vector<char> buffer;

					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::LoginClient::DISCONNECT, buffer);
					Java::Packet::writeString(reason, buffer);
					server->sendDataToClient(client, buffer);
					server->disconnectClient(client.get());
				}

				void loginStart(std::shared_ptr<::Client> client, Watermelon* server, const std::string& username, std::pair<uint64_t, uint64_t> uuid) {
					std::vector<char> buffer;
					std::pair<uint64_t, uint64_t> sessionUUID = Utils::UUID::generateSessionUUID();

					if (!client->createPlayer(username, uuid, server->increaseEntityCounter(), { 0, 0, 0 }, { 0, 0 })) {
						json reason = {
							{ "translate", "multiplayer.disconnect.unverified_username" }
						};

						return disconnect(client, server, reason.dump());
					}
					server->getLogger().log("UUID of player " + username + " is " + Utils::UUID::formatUUID(uuid), "Network", Logger::INFO);
					client->setSessionUUID(sessionUUID);

					if (server->getConfig()["networking"]["java"]["online_mode"].value_or(true)) {
						// encryption packet
					} else {
						Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::LoginClient::LOGIN_SUCCESS, buffer);
						Java::Packet::writeUUID(uuid, buffer);
						Java::Packet::writeString(username, buffer);
						Java::Packet::writeVarInt(0, buffer);
						Java::Packet::writeUUID(sessionUUID, buffer);
					}

					server->sendDataToClient(client, buffer);
				}
			};
		};
	};
};
