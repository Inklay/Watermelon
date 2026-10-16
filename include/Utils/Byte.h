#pragma once

#include <bit>
#include <cstdint>

namespace Byte {
	template <typename T>
	T toBigEndian(T value) {
		if constexpr (std::endian::native == std::endian::little) {
			return std::byteswap(value);
		}
		else {
			return value;
		}
	}
}
