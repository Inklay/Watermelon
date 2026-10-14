#include "Java/Packet/Packet.h"
#include "Registry/RegistryManager.h"
#include "Java/Packet/Client/StatusPackets.h"

namespace Java {
	namespace Packet {
		namespace Client {
			namespace Play {
				void synchronizePlayerPosition(std::shared_ptr<::Client> client, Watermelon* server) {
					bool wasTeleporting = false;

					{
						std::lock_guard<std::mutex> lock(server->getClientMutex());
						wasTeleporting = client->teleporting;
					}

					if (wasTeleporting) {
						return;
					}

					std::vector<char> buffer;
					int32_t teleportId = 0;

					{
						std::lock_guard<std::mutex> lock(server->getClientMutex());
						teleportId = client->teleportIdCounter;
						client->teleporting = true;
					}

					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::PlayClient::SYNCHRONIZE_PLAYER_POSITION, buffer);
					Java::Packet::writeVarInt(teleportId, buffer);
					Java::Packet::writeDouble(0, buffer); // FIXME x
					Java::Packet::writeDouble(0, buffer); // FIXME y
					Java::Packet::writeDouble(0, buffer); // FIXME z
					Java::Packet::writeDouble(0, buffer);
					Java::Packet::writeDouble(0, buffer);
					Java::Packet::writeDouble(0, buffer);
					Java::Packet::writeFloat(0, buffer); // FIXME yaw
					Java::Packet::writeFloat(0, buffer); // FIXME pitch
					Java::Packet::write32(0, buffer);

					server->sendDataToClient(client, buffer);
				}

				void loginPacket(std::shared_ptr<::Client> client, Watermelon* server) {
					std::cout << "ici -1" << std::endl;
					std::vector<char> buffer;
					const Registry* dimensionReg = RegistryManager::getInstance().getRegistry("minecraft:dimension");

					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::PlayClient::LOGIN, buffer);
					Java::Packet::writeVarInt(server->increaseEntityCounter(), buffer);
					Java::Packet::writeBool(false, buffer); // FIXME is hardcore
					Java::Packet::writeVarInt((int32_t)dimensionReg->entries.size(), buffer);
					std::cout << "ici 0" << std::endl;
					for (const Registry::Entry& entry : dimensionReg->entries) {
						Java::Packet::writeString(entry.textId, buffer);
					}

					std::cout << "ici 1" << std::endl;

					Java::Packet::writeVarInt(server->getConfig()["networking"]["java"]["max_players"].value_or(10), buffer);
					Java::Packet::writeVarInt(server->getConfig()["networking"]["java"]["view_distance"].value_or(16), buffer);
					Java::Packet::writeVarInt(server->getConfig()["networking"]["java"]["simulation_distance"].value_or(10), buffer);
					Java::Packet::writeBool(false, buffer);
					Java::Packet::writeBool(true, buffer); // FIXME enable respawn screen
					Java::Packet::writeBool(false, buffer);
					Java::Packet::writeVarInt(dimensionReg->entries.at(0).protocolId, buffer); // FIXME dimension type
					Java::Packet::writeString(dimensionReg->entries.at(0).textId, buffer); // FIXME dimension name
					Java::Packet::write64(0, buffer); // FIXME hashed seed
					Java::Packet::writeVarInt(Java::Packet::Gamemode::CREATIVE, buffer); // FIXME game mode
					std::cout << "ici 2" << std::endl;
					Java::Packet::writeVarInt(-1, buffer); // FIXME previous game mode
					Java::Packet::writeBool(false, buffer); // FIXME is debug
					Java::Packet::writeBool(false, buffer); // FIXME is sflat
					Java::Packet::writeBool(false, buffer); // FIXME has death location
					Java::Packet::writeVarInt(0, buffer);
					Java::Packet::writeVarInt(63, buffer);
					Java::Packet::writeBool(server->getConfig()["networking"]["java"]["online_mode"].value_or(true), buffer);
					Java::Packet::writeBool(false, buffer); // FIXME enforce secure chat

					std::cout << "ici 3" << std::endl;
					server->sendDataToClient(client, buffer);
				}
			};
		};
	};
};
