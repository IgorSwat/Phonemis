#pragma once

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <functional>
#include <string>
#include <tuple>
#include <vector>

namespace phonemis::test {

  // ---------------
	// Test definition
	// ---------------

  // Test is a pair containing test name and test function (simple void function)
  using Test = std::pair<std::string, std::function<bool()>>;


  // ----------------
	// Global test list
	// ----------------

    // Global list that aggregates all tests created with REGISTER_TEST macro
    inline std::vector<Test>& get_tests() 
    {
        static std::vector<Test> tests;

        return tests;
    }

  // -------------
	// Test creation
	// -------------

  // This function allows for compile-time creating and aggregating tests
  #define REGISTER_TEST(name) \
    bool name(); \
    struct name##_t { \
        name##_t() { get_tests().emplace_back(#name, name); } \
    }; \
    static name##_t name##_instance; \
    bool name()
    

  // -------------------
	// Test error printing
	// -------------------

  // Color defines
  // - Create more readable UI during test run
  // - Uses ANSI macros for displaying colors in console
  #define COLOR_RESET   "\033[0m"
  #define COLOR_RED     "\033[31m"
  #define COLOR_GREEN   "\033[32m"
  #define COLOR_YELLOW  "\033[33m"
  #define COLOR_BLUE    "\033[34m"

  // A custom ASSERT macro combines the effect of standard assert and printing the error message
  // - Prints (msg) message if condition fails
  #define ASSERT_EQUALS(expected, real) \
    do { \
      if (expected != real) { \
        std::cerr << COLOR_RED "Test failed: " COLOR_RESET; \
        std::cerr << "File: " << __FILE__ << ", Line: " << __LINE__ << " - "; \
        std::cerr << "expected " << expected << ", got " << real << "\n"; \
        return false; \
      } \
    } while (0)


  // ------------
	// Test running
	// ------------

  // Perform all the tests
  // - Returns true if all tests have succeeded and false if even one test went wrong
  bool run_tests();

} // namespace phonemis::test 