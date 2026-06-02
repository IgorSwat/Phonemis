#pragma once

#include <numeric>
#include <vector>

namespace phonemis::protophone::numeric {

/**
 * Calculates the product of elements in a vector.
 * Returns 0 for empty vectors.
 * @param values The vector containing elements to multiply.
 */
template <typename T>
inline T product(const std::vector<T>& values) {
  if (values.empty()) {
    return static_cast<T>(0);
  }

  return std::accumulate(
    values.begin(), 
    values.end(), 
    static_cast<T>(1), std::multiplies<T>()
  );
}

} // namespace phonemis::protophone::numeric
