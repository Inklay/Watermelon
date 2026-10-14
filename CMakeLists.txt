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
	source/Logger.cpp
    source/Player.cpp
	source/Config.cpp
	source/Watermelon.cpp
    source/Utils/UUID.cpp
    source/Java/Packet/Packet.cpp
    source/Java/Packet/Server/Parser.cpp
    source/Java/Packet/Server/HandshakePackets.cpp
    source/Java/Packet/Server/StatusPackets.cpp
    source/Java/Packet/Server/LoginPackets.cpp
    source/Java/Packet/Server/ConfigurationPackets.cpp
    source/Java/Packet/Client/StatusPackets.cpp
    source/Java/Packet/Client/LoginPackets.cpp
    source/Java/Packet/Client/ConfigurationPackets.cpp
 "source/Registry/RegistryManager.cpp" "source/Client.cpp")

add_executable(Watermelon ${SOURCES})

# ---------------------------------------------------------------------------
# Include directories
# ---------------------------------------------------------------------------
target_include_directories(Watermelon PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# ---------------------------------------------------------------------------
# Additional librairies
# ---------------------------------------------------------------------------
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()

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
