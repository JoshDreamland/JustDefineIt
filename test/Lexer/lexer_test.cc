#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <System/lex_cpp.h>
#include <Testing/error_handler.h>
#include <Testing/matchers.h>

#define TOKEN(type) token_t(type, __FILE__, __LINE__, 0)

namespace jdi {

TEST(LexerTest, BasicTokenization) {
  macro_map no_macros;
  llreader read("test_input", R"cpp(const char *my_identifier = "hello, world!";
      )cpp", false);
  lexer lex(read, no_macros, error_constitutes_failure);
  
  EXPECT_THAT(lex.get_token(), HasType(TT_DECFLAG));       // const
  EXPECT_THAT(lex.get_token(), HasType(TT_DECLARATOR));    // char
  EXPECT_THAT(lex.get_token(), HasType(TT_STAR));          // *
  EXPECT_THAT(lex.get_token(), HasType(TT_IDENTIFIER));    // my_identifier
  EXPECT_THAT(lex.get_token(), HasType(TT_EQUAL));         // =
  EXPECT_THAT(lex.get_token(), HasType(TT_STRINGLITERAL)); // "hello, world!"
  EXPECT_THAT(lex.get_token(), HasType(TT_SEMICOLON));     // ;
  EXPECT_THAT(lex.get_token(), HasType(TT_ENDOFCODE));
}
TEST(LexerTest, StringLiteralBehavior) {
  macro_map no_macros;
  llreader read("test_input", R"cpp("hello,"    ""    " world!")cpp", false);
  lexer lex(read, no_macros, error_constitutes_failure);
  
  EXPECT_THAT(lex.get_token(), HasType(TT_STRINGLITERAL));
  EXPECT_THAT(lex.get_token(), HasType(TT_STRINGLITERAL));
  EXPECT_THAT(lex.get_token(), HasType(TT_STRINGLITERAL));
  EXPECT_THAT(lex.get_token(), HasType(TT_ENDOFCODE));
}

// Ridiculously directive-heavy code that says `int identifier = 4;`
constexpr char kCppWithIfDirectives[] = R"cpp(
#if 1 + 1 == 2
  int
#else
  wrong
#endif

#if 2 + 2 == 5
  decltyperrr
#elif 2 + 2 == 4
  identifier =
#else
  sizeof
#endif

4;
)cpp";
TEST(LexerTest, BasicPreprocessors) {
  macro_map no_macros;
  llreader read("test_input", kCppWithIfDirectives, false);
  lexer lex(read, no_macros, error_constitutes_failure);
  
  EXPECT_THAT(lex.get_token(), HasType(TT_DECLARATOR));    // int
  EXPECT_THAT(lex.get_token(), HasType(TT_IDENTIFIER));    // identifier
  EXPECT_THAT(lex.get_token(), HasType(TT_EQUAL));         // =
  EXPECT_THAT(lex.get_token(), HasType(TT_DECLITERAL));    // 4
  EXPECT_THAT(lex.get_token(), HasType(TT_SEMICOLON));     // ;
  EXPECT_THAT(lex.get_token(), HasType(TT_ENDOFCODE));
}

// Macro-heavy code that says `int identifier = 100;`
constexpr char kCppWithMacroFunc[] = R"cpp(
#define MACRO_FUNC(x, y, type) type x = y
MACRO_FUNC(identifier, 100, int);
)cpp";
TEST(LexerTest, BasicMacroFunction) {
  macro_map no_macros;
  llreader read("test_input", kCppWithMacroFunc, false);
  lexer lex(read, no_macros, error_constitutes_failure);
  
  EXPECT_THAT(lex.get_token(), HasType(TT_DECLARATOR));    // int
  EXPECT_THAT(lex.get_token(), HasType(TT_IDENTIFIER));    // identifier
  EXPECT_THAT(lex.get_token(), HasType(TT_EQUAL));         // =
  EXPECT_THAT(lex.get_token(), HasType(TT_DECLITERAL));    // 100
  EXPECT_THAT(lex.get_token(), HasType(TT_SEMICOLON));     // ;
  EXPECT_THAT(lex.get_token(), HasType(TT_ENDOFCODE));
}

// Macro-heavy code that says `int x;`
constexpr char kCppWithConditionalExpansion[] = R"cpp(
#define butts 12345
#if 2 > 1 && butts == 12345
  int x;
#endif
)cpp";
TEST(LexerTest, ConditionalWithMacroExpansion) {
  macro_map no_macros;
  llreader read("test_input", kCppWithConditionalExpansion, false);
  lexer lex(read, no_macros, error_constitutes_failure);
  
  EXPECT_THAT(lex.get_token(), HasType(TT_DECLARATOR));    // int
  EXPECT_THAT(lex.get_token(), HasType(TT_IDENTIFIER));    // x
  EXPECT_THAT(lex.get_token(), HasType(TT_SEMICOLON));     // ;
  EXPECT_THAT(lex.get_token(), HasType(TT_ENDOFCODE));
}

// Tests a simple not expression with a macro.
constexpr char kCppWithNot[] = R"cpp(
#define butts 1
#if ! butts
uh_oh
#endif
)cpp";
TEST(LexerTest, ConditionalWithNot) {
  macro_map no_macros;
  llreader read("test_input", kCppWithNot, false);
  lexer lex(read, no_macros, error_constitutes_failure);
  EXPECT_THAT(lex.get_token(), HasType(TT_ENDOFCODE));
}

}  // namespace jdi
