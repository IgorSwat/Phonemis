#include "test.h"
#include <iomanip>

namespace phonemis::test {

  bool run_tests()
  {
    auto& tests = get_tests();
    std::cout << COLOR_BLUE "Running " << tests.size() << " test(s)...\n" COLOR_RESET << std::string(40, '-') << "\n";

    size_t passed = 0;
    size_t failed = 0;

    // Perform every test once
    for (const auto& [name, test_fn] : tests) {
        std::cout << COLOR_BLUE "Running " << name << "..." COLOR_RESET "\n";

        if (test_fn()) {
            std::cout << COLOR_GREEN "Test passed!\n" COLOR_RESET;
            passed++;
        } else {
            failed++;
        }
    }

    std::cout << std::string(40, '-') << "\n";
    std::cout << COLOR_BLUE "All tests finished.\n" COLOR_RESET;
    
    if (failed == 0) {
        std::cout << COLOR_GREEN "Summary: " << passed << " passed, 0 failed.\n" COLOR_RESET;
    } else {
        std::cout << COLOR_RED "Summary: " << passed << " passed, " << failed << " failed.\n" COLOR_RESET;
    }

    return failed == 0;
  }

} // namespace phonemis::test

int main() {
  return (phonemis::test::run_tests() ? 0 : 1);
}
