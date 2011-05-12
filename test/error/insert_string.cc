#include <string>
#include <string.h>
#include "error.hh"

using namespace com::centreon::engine;

/**
 *  Check that std::string insertion works with error.
 *
 *  @return 0 on success.
 */
int main() {
  // Strings.
  std::string s1("foo");
  std::string s2(" bar baz");
  std::string s3(" qux");

  // Insert strings.
  error e;
  e << s1 << s2;
  e << s3;

  // Check.
  return (strcmp("foo bar baz qux", e.what()));
}
