#include "Java/Parser/HandshakePackets.h"

namespace Java {
	namespace Parser {
		namespace LoginPackets {
			static void loginStart(std::shared_ptr<Client> client, const Java::Packet::Header& header, Watermelon* server) {
			}

			void parse(std::shared_ptr<Client> client, const Java::Packet::Header& header, Watermelon* server) {
				switch (header.packetId) {
					case Java::Packet::PacketId::Login::LOGIN_START:
						return loginStart(client, header, server);
				}
			}
		};
	};
};
