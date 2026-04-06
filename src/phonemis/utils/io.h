#pragma once

#include "conversions.h"
#include <third-party/json.hpp>

#include <iostream>
#include <string>

/**
 * Input/Output utilities
 * 
 * A set of utilities to handle filesystem interactions.
 */
namespace phonemis {
namespace utils::io {

/**
 * JSON file parsing - a decorator for external nlohmann::json parser.
 * @param fp The file path to the JSON file.
 * @return The parsed nlohmann::json object.
 * @throws std::invalid_argument If the file is not found or the JSON format is invalid.
 * @throws std::runtime_error If the file fails to open.
 */
nlohmann::json load_json(const std::string& fp);

}	// utils::io

/**
 * Custom IO overloads for u32string
 */
inline std::ostream& operator<<(std::ostream& os, const std::u32string& u32) {
  return os << phonemis::utils::conversions::u32_to_utf8(u32);
}

inline std::ostream& operator<<(std::ostream& os, const char32_t* u32) {
  return os << phonemis::utils::conversions::u32_to_utf8(u32);
}

} // phonemis::utilities::io