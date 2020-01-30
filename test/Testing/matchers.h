#ifndef JDI_TESTING_MATCHERS_h
#define JDI_TESTING_MATCHERS_h

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "System/token.h"

namespace {
    
using ::testing::Matcher;
using ::testing::MatcherInterface;
using ::testing::MatchResultListener;

class HasTypeMatcher : public MatcherInterface<const jdi::token_t&> {
  jdi::TOKEN_TYPE type_;

 public:
  explicit HasTypeMatcher(jdi::TOKEN_TYPE type): type_(type) {}

  bool MatchAndExplain(const jdi::token_t &token,
                       MatchResultListener* /* listener */) const override {
    return token.type == type_;
  }

  void DescribeTo(::std::ostream* os) const override {
    *os << "token type is " << type_;
  }

  void DescribeNegationTo(::std::ostream* os) const override {
    *os << "token type is not " << type_;
  }
};

class HasContentMatcher : public MatcherInterface<const jdi::token_t&> {
  std::string_view content_;

 public:
  explicit HasContentMatcher(std::string_view content): content_(content) {}

  bool MatchAndExplain(const jdi::token_t &token,
                       MatchResultListener* /* listener */) const override {
    return token.content.view() == content_;
  }

  void DescribeTo(::std::ostream* os) const override {
    *os << "token content is `" << content_ << '`';
  }

  void DescribeNegationTo(::std::ostream* os) const override {
    *os << "token content is not `" << content_ << '`';
  }
};

Matcher<const jdi::token_t&> HasType(jdi::TOKEN_TYPE type) {
  return MakeMatcher(new HasTypeMatcher(type));
}

Matcher<const jdi::token_t&> HasContent(std::string_view type) {
  return MakeMatcher(new HasContentMatcher(type));
}

}  // namespace

#endif  // JDI_TESTING_MATCHERS_h
