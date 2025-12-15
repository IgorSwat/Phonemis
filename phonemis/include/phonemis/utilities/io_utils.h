#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <third-party/json.hpp>

namespace phonemis::utilities::io_utils {

// JSON file parsing
// A decorator for external nlohmann::json parser.
// Note that it loads an entire JSON to the memory, so its not recommended
// for very large JSON files.
inline nlohmann::json load_json(const std::string& fp) {
  std::filesystem::path file_path(fp);
	if (!std::filesystem::exists(file_path) || !std::filesystem::is_regular_file(file_path)) {
		throw std::invalid_argument("File not found: " + fp);
	}

	// JSON parsing
	std::ifstream file_stream(fp);
	if (!file_stream.is_open()) {
		throw std::runtime_error("Failed to open file: " + fp);
	}

	nlohmann::json json_obj;
	try {
		file_stream >> json_obj;
	} catch (const nlohmann::json::parse_error& e) {
		throw std::invalid_argument(std::string("Invalid JSON format: ") + e.what());
	}

  return json_obj;
}

} // phonemis::utilities::io