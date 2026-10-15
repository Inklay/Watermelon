#include "Utils/UUID.h"
#include "Java/Packet/Packet.h"
#include "Java/Packet/Client/StatusPackets.h"

namespace Java {
	namespace Packet {
		namespace Client {
			namespace Login {
				void loginStartPacket(std::shared_ptr<::Client> client, Watermelon* server, const std::string& username, std::pair<uint64_t, uint64_t> uuid) {
					std::vector<char> buffer;
					std::pair<uint64_t, uint64_t> sessionUUID = Utils::UUID::generateSessionUUID();

					server->getLogger().log("UUID of player " + username + " is " + Utils::UUID::formatUUID(uuid), "Network", Logger::INFO);
					client->setSessionUUID(sessionUUID);
					client->createPlayer(username, uuid, server->increaseEntityCounter(), { 0, 0, 0 }, { 0, 0 });

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
