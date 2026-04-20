#include "io.h"

#include <filesystem>
#include <fstream>
#include <string>

namespace phonemis::utils::io {

nlohmann::json load_json(std::string_view fp) {
  std::filesystem::path file_path(fp);
	if (!std::filesystem::exists(file_path) || !std::filesystem::is_regular_file(file_path)) {
		throw std::invalid_argument("File not found: " + std::string(fp));
	}

	// JSON parsing
	std::ifstream file_stream(file_path);
	if (!file_stream.is_open()) {
		throw std::runtime_error("Failed to open file: " + std::string(fp));
	}

	nlohmann::json json_obj;
	try {
		file_stream >> json_obj;
	} catch (const nlohmann::json::parse_error& e) {
		throw std::invalid_argument(std::string("Invalid JSON format: ") + e.what());
	}

  return json_obj;
}

} // namespace phonemis::utils