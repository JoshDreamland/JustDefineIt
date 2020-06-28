#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <System/lex_cpp.h>
#include <System/builtins.h>
#include <Testing/error_handler.h>
#include <Testing/matchers.h>

using ::testing::Eq;
using ::testing::AllOf;

namespace jdi {
namespace {

Context Parse(const char *test_case) {
  Context ctex(error_constitutes_failure);
  macro_map no_macros;
  llreader read("test_input", test_case, false);
  ctex.parse_stream(read);
  return ctex;
}

TEST(ParsingTest, VariousDeclarations) {
  Parse(R"cpp(
    int integer;
    char *cstring;
    int intfn(int x);
    int (*intfn_ptr)(int);
  )cpp");
}

TEST(ParsingTest, HighlyDecoratedIntegers) {
  Parse("long long ago;                     ");
  Parse("const long unsigned long int etc;  ");
}

TEST(ParsingTest, HighlyDecoratedIntegerTypedefs) {
  Parse("typedef long long ago;                     ");
  Parse("typedef const long unsigned long int etc;  ");
}

}  // namespace
}  // namespace jdi
