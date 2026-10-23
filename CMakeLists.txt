cmake_minimum_required (VERSION 3.20)
project ("Watermelon" LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------
set(SOURCES
	source/main.cpp
	source/Core/Logger.cpp
    source/Core/Keys.cpp
	source/Core/Config.cpp
    source/Core/Client.cpp
	source/Core/Watermelon.cpp
    source/Core/TaskPool.cpp
    source/Auth/Auth.cpp
    source/Utils/UUID.cpp
    source/Utils/Buffer.cpp
    source/Utils/Base64.cpp
    source/Entity/Entity.cpp
    source/Entity/EntityManager.cpp
    source/Player/Player.cpp
    source/Player/ProfileProperty.cpp
    source/Player/ChatSession.cpp
    source/Java/Packet/Packet.cpp
    source/Java/Packet/Server/Handler.cpp
    source/Java/Packet/Server/HandshakePackets.cpp
    source/Java/Packet/Server/StatusPackets.cpp
    source/Java/Packet/Server/LoginPackets.cpp
    source/Java/Packet/Server/ConfigurationPackets.cpp
    source/Java/Packet/Server/PlayPackets.cpp
    source/Java/Packet/Client/StatusPackets.cpp
    source/Java/Packet/Client/LoginPackets.cpp
    source/Java/Packet/Client/ConfigurationPackets.cpp
    source/Java/Packet/Client/PlayPackets.cpp
    source/Registry/RegistryManager.cpp
    source/Crypto/RSA.cpp
    source/Crypto/CFB8.cpp
    source/Crypto/Crypto.cpp
    source/Compression/ZLIB.cpp
)

add_executable(Watermelon ${SOURCES})

# ---------------------------------------------------------------------------
# Include directories
# ---------------------------------------------------------------------------
target_include_directories(Watermelon PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/stubs
)

# ---------------------------------------------------------------------------
# Additional librairies
# ---------------------------------------------------------------------------
if(WIN32)
    target_compile_definitions(Watermelon PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()

find_package(OpenSSL 3.0 REQUIRED)
find_package(httplib CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(tomlplusplus CONFIG REQUIRED)

target_link_libraries(Watermelon PRIVATE 
    OpenSSL::Crypto
    OpenSSL::SSL
    httplib::httplib
    ZLIB::ZLIB
    nlohmann_json::nlohmann_json
    tomlplusplus::tomlplusplus
)

# ---------------------------------------------------------------------------
# Compiler options
# ---------------------------------------------------------------------------
target_compile_options(Watermelon PRIVATE
    # UTF-8 source & execution charset (MSVC) / equivalent for GCC/Clang
    $<IF:$<CXX_COMPILER_ID:MSVC>,
        /W3 /WX /utf-8 /MP,
        -Wall -Wextra -Werror -finput-charset=UTF-8>
)

# Add MSVC Hot reload
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# ---------------------------------------------------------------------------
# Preprocessor definitions
# ---------------------------------------------------------------------------
target_compile_definitions(Watermelon PRIVATE
    UNICODE _UNICODE
    $<$<CONFIG:Debug>:_DEBUG _CONSOLE>
)

# ---------------------------------------------------------------------------
# Link-time optimisation (Release only — mirrors WholeProgramOptimization)
# ---------------------------------------------------------------------------
set_target_properties(Watermelon PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE ON
)

# ---------------------------------------------------------------------------
# Stub generated header file if needed
# ---------------------------------------------------------------------------

function(stubRegistries header prefix)
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/include/Registry/${header}")
        file(WRITE "${CMAKE_BINARY_DIR}/stubs/Registry/${header}"
"#pragma once
#include <array>
#include <cstddef>
#include \"Registry/Registry.h\"

namespace Registries {
	inline const std::array<const Registry*, 0> ${prefix}_REGISTRIES = {};
	inline const std::size_t ${prefix}_REGISTRIES_COUNT = 0;
}
")
        message(WARNING "include/Registry/${header} is missing: using an empty stub, clients won't be able to join. Run the scripts in codegen/ (see README).")
    endif()
endfunction()

stubRegistries(BuiltIn.h BUILT_IN)
stubRegistries(Synchronised.h SYNCHRONIZED)

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/include/Tag/Tags.h")
    file(WRITE "${CMAKE_BINARY_DIR}/stubs/Tag/Tags.h"
"#pragma once

#include <array>
#include \"Tag/RegistryTag.h\"

namespace Tags {
	inline const std::array<const RegistryTag*, 0> REGISTRY_TAGS = {};
	inline const std::size_t REGISTRY_TAGS_COUNT = 0;
}
")
    message(WARNING "include/Tag/Tags.h is missing: using an empty stub, clients won't be able to join. Run the scripts in codegen/ (see README).")
endif()
