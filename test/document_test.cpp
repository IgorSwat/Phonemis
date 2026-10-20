#include "test.h"
#include <phonemis/base/processor/common/document.h>
#include <phonemis/utils/io.h>

#include <stdexcept>

namespace phonemis::test {

using namespace processor;

namespace {
// Whether a call rejects its arguments
template <typename F>
bool throws_invalid_argument(F call) {
  try {
    call();
  } catch (const std::invalid_argument&) {
    return true;
  }
  return false;
}
} // namespace

REGISTER_TEST(document_commit_test) {
  Document document(U"I have 2 cats and 3 dogs");

  // Edits are queued in any order, with spans of the text before the commit
  document.replace({18, 19}, U"three");
  document.replace({7, 8}, U"two");
  ASSERT_EQUALS(U"I have 2 cats and 3 dogs", std::u32string(document.text()));

  document.commit();
  ASSERT_EQUALS(U"I have two cats and three dogs", std::u32string(document.text()));

  // Deletions and insertions
  document.replace({0, 2}, U"");
  document.replace({6, 6}, U" got");
  document.commit();
  ASSERT_EQUALS(U"have got two cats and three dogs", std::u32string(document.text()));

  return true;
}

REGISTER_TEST(document_invalid_edits_test) {
  Document document(U"abcdef");
  document.replace({1, 3}, U"x");

  ASSERT_EQUALS(true, throws_invalid_argument([&] { document.replace({2, 4}, U"y"); }));
  ASSERT_EQUALS(true, throws_invalid_argument([&] { document.replace({0, 2}, U"y"); }));
  ASSERT_EQUALS(true, throws_invalid_argument([&] { document.replace({4, 7}, U"y"); }));
  ASSERT_EQUALS(false, throws_invalid_argument([&] { document.replace({3, 4}, U"y"); }));

  return true;
}

REGISTER_TEST(document_changes_test) {
  // Changes of several commits map to the original text
  Document document(U"10 km away");
  document.replace({0, 2}, U"ten");
  document.commit();
  document.replace({4, 6}, U"kilometers");
  document.commit();
  ASSERT_EQUALS(U"ten kilometers away", std::u32string(document.text()));

  auto changes = document.changes();
  ASSERT_EQUALS(2, changes.size());
  ASSERT_EQUALS(0, changes[0].span.begin);
  ASSERT_EQUALS(2, changes[0].span.end);
  ASSERT_EQUALS(U"ten", changes[0].text);
  ASSERT_EQUALS(3, changes[1].span.begin);
  ASSERT_EQUALS(5, changes[1].span.end);
  ASSERT_EQUALS(U"kilometers", changes[1].text);

  return true;
}

REGISTER_TEST(document_widened_edit_test) {
  // An edit inside a replaced piece replaces all of it
  Document document(U"10 km");
  document.replace({0, 2}, U"ten");
  document.commit();
  document.replace({0, 1}, U"T");
  document.commit();
  ASSERT_EQUALS(U"Ten km", std::u32string(document.text()));

  auto changes = document.changes();
  ASSERT_EQUALS(1, changes.size());
  ASSERT_EQUALS(0, changes[0].span.begin);
  ASSERT_EQUALS(2, changes[0].span.end);
  ASSERT_EQUALS(U"Ten", changes[0].text);

  return true;
}

} // namespace phonemis::test
