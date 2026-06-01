#pragma once

#include "types.h"
#include <cstddef>
#include <type_traits>
#include <variant>

namespace phonemis::protophone::meta {

/**
 * Converts a slicing argument to a canonical index or range variant.
 * Maps integral types to size_t.
 */
template <typename T>
auto to_slice_variant(T&& arg) -> std::variant<size_t, Range<size_t>, All> {
  using V = std::decay_t<T>;
  if constexpr (std::is_integral_v<V>) {
    return static_cast<size_t>(arg);
  } else {
    return std::forward<T>(arg);
  }
}

} // namespace phonemis::protophone::meta
