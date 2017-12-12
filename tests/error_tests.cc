#include <string>

#include <gtest/gtest.h>

#include "error.h"

TEST(ErrorTests, Warn) {
  cyclus::Warn<cyclus::KEY_WARNING>("my spoon is too big!");
}

TEST(ErrorTests, WarnAsError) {
  cyclus::warn_as_error = true;
  EXPECT_THROW(cyclus::Warn<cyclus::VALUE_WARNING>("I'm a banana!"),
               cyclus::ValueError);
  cyclus::warn_as_error = false;
}

TEST(ErrorTests, WarnLimit) {
  using cyclus::warn_limit;
  unsigned int prev = warn_limit;
  warn_limit = 0;
  cyclus::Warn<cyclus::KEY_WARNING>("my spoon is toooooo big!");
  warn_limit = prev;
}

