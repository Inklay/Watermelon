#include <filesystem>
#include "Core/Config.h"
#include <fstream>

toml::table loadConfig() {
	if (std::filesystem::exists("watermelon.toml")) {
		return toml::parse_file("watermelon.toml");
	} else {
		toml::table config = toml::parse(R"(
			#seed = "1790155153199293024"
			#default_difficulty = "Normal"
			#op_permission_level = 4
			#allow_nether = true
			#allow_end = true
			hardcore = false
			tps = 20.0
			default_gamemode = "Survival"
			force_gamemode = false
			#scrub_ips = true
			use_favicon = true
			favicon_path = "icon.png"
			#default_level_name = "world"
			#allow_chat_reports = false
			#white_list = false
			#enforce_whitelist = false

			[logging]
			enabled = true
			level = "info"
			threads = true
			thread_ids = false
			target = true
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
			#keep_alive_time = 15
			motd = "A (hopefully) blazingly fast Watermelon server!"
		)");

		std::ofstream("watermelon.toml", std::ios::out | std::ios::trunc) << config;

		return config;
	}
}
