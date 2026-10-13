#include <filesystem>
#define TOML_IMPLEMENTATION
#include "Config.h"

toml::table loadConfig() {
	if (std::filesystem::exists("watermelon.toml")) {
		return toml::parse_file("watermelon.toml");
	} else {
		toml::table config = toml::parse(R"(
			[logging]
			enabled = true
			level = "info"
			threads = false
			thread_ids = false
			target = false
			color = true
			timestamp = true
			timestamp_format = "[hour]:[minute]:[second]"
			file = "latest.log"

			[networking.java]
			enabled = true
			address = "0.0.0.0:25565"
			encryption = true
			online_mode = true
			max_players = 10
			view_distance = 16
			simulation_distance = 10
			keep_alive_time = 15
			motd = "A (hopefully) blazingly fast Watermelon server!"
		)");

		std::ofstream("watermelon.toml", std::ios::out | std::ios::trunc) << config;

		return config;
	}
}
