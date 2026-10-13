#pragma once

#include <mutex>
#include <memory>
#include <vector>
#include "../Client.h"
#include "../Logger.h"

namespace Java {
	namespace Packet {
		enum Intent {
			STATUS = 1,
			LOGIN = 2,
			TRANSFER = 3
		};
		namespace PacketId {
			enum Handshake {
				HANDSHAKE = 0x00
			};

			enum Status {
				STATUS = 0x00,
				PING = 0x01
			};

			enum Login {
				LOGIN_START = 0x00
			};
		};

		struct Header {
			int32_t length;
			int32_t packetId;
		};

		int32_t readVarInt(std::shared_ptr<Client> client, uint32_t& offset);
		std::string readString(std::shared_ptr<Client> client, uint32_t& offset);
		uint16_t readU16(std::shared_ptr<Client> client, uint32_t& offset);
		int64_t read64(std::shared_ptr<Client> client, uint32_t& offset);

		void writeVarInt(std::shared_ptr<Client> client, int32_t value, std::vector<char>& buffer);
		void write64(std::shared_ptr<Client> client, int64_t value, std::vector<char>& buffer);
		void writeString(std::shared_ptr<Client> client, const std::string& str, std::vector<char>& buffer);
	};
}
