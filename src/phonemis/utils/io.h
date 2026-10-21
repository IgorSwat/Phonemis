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
nlohmann::json load_json(std::string_view fp);

/**
 * JSON file parsing with a callback, which sees every element as it is parsed and can
 * keep it out of the result, to avoid holding a large file in memory twice.
 * @param fp The file path to the JSON file.
 * @param callback The nlohmann::json parser callback; returning false drops the element.
 * @return The parsed nlohmann::json object, without the dropped elements.
 * @throws std::invalid_argument If the file is not found or the JSON format is invalid.
 * @throws std::runtime_error If the file fails to open.
 */
nlohmann::json load_json(std::string_view fp, const nlohmann::json::parser_callback_t& callback);

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

} // phonemis