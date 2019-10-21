#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <System/lex_cpp.h>
#include <Testing/error_handler.h>
#include <Testing/matchers.h>

#define TOKEN(type) token_t(type, __FILE__, __LINE__, 0)

using ::testing::Eq;

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

TEST(LexerTest, ConcatenationInObjectLikeMacros) {
  constexpr char kTestCase[] = R"(
    #define type in ## t
    type name;
  )";
  macro_map no_macros;
  llreader read("test_input", kTestCase, false);
  lexer lex(read, no_macros, error_constitutes_failure);
  EXPECT_THAT(lex.get_token(), HasType(TT_DECLARATOR));    // int
  EXPECT_THAT(lex.get_token(), HasType(TT_IDENTIFIER));    // name
  EXPECT_THAT(lex.get_token(), HasType(TT_SEMICOLON));     // ;
  EXPECT_THAT(lex.get_token(), HasType(TT_ENDOFCODE));
}

TEST(LexerTest, UncalledMacroFuncLeftAlone) {
  constexpr char kTestCase[] = R"(
    #define macro_func(x)
    int macro_func = 10;
  )";

  macro_map no_macros;
  llreader read("test_input", kTestCase, false);
  lexer lex(read, no_macros, error_constitutes_failure);
  EXPECT_THAT(lex.get_token(), HasType(TT_DECLARATOR));    // int
  EXPECT_THAT(lex.get_token(), HasType(TT_IDENTIFIER));    // macro_func
  EXPECT_THAT(lex.get_token(), HasType(TT_EQUAL));         // =
  EXPECT_THAT(lex.get_token(), HasType(TT_DECLITERAL));    // 10
  EXPECT_THAT(lex.get_token(), HasType(TT_SEMICOLON));     // ;
  EXPECT_THAT(lex.get_token(), HasType(TT_ENDOFCODE));
}

TEST(LexerTest, ISO_n4727_19_3_3) {
  constexpr char kTestCase[] = R"(
    #define hash_hash # ## #
    #define mkstr(a) # a
    #define in_between(a) mkstr(a)
    #define join(c, d) in_between(c hash_hash d)
    join(x, y)
  )";

  macro_map no_macros;
  llreader read("test_input", kTestCase, false);
  lexer lex(read, no_macros, error_constitutes_failure);

  token_t token = lex.get_token();
  ASSERT_THAT(token, HasType(TT_STRINGLITERAL));
  EXPECT_THAT(token.content.toString(), Eq("\"x ## y\""));
  EXPECT_THAT(lex.get_token(), HasType(TT_ENDOFCODE));
}

TEST(LexerTest, AnnoyingSubstitutionOrder) {
  constexpr char kTestCase[] = R"(
    #define cat1(x, y) x ## y
    #define cat2(x, y) cat1(x, y)
    #define identifier cat1(id, __LINE__)
    cat1(id, __LINE__)
    identifier
    cat2(id, __LINE__)
  )";

  macro_map no_macros;
  llreader read("test_input", kTestCase, false);
  lexer lex(read, no_macros, error_constitutes_failure);

  token_t token = lex.get_token();
  ASSERT_THAT(token, HasType(TT_IDENTIFIER));
  EXPECT_THAT(token.content.toString(), Eq("id__LINE__"));

  token = lex.get_token();
  ASSERT_THAT(token, HasType(TT_IDENTIFIER));
  EXPECT_THAT(token.content.toString(), Eq("id__LINE__"));

  token = lex.get_token();
  ASSERT_THAT(token, HasType(TT_IDENTIFIER));
  EXPECT_THAT(token.content.toString(), Eq("id6"));

  EXPECT_THAT(lex.get_token(), HasType(TT_ENDOFCODE));
}

TEST(LexerTest, InducesTears) {
  constexpr char kTestCase[] = R"(
    #define a(x) # x
    #define b(x) a ( # x )
    #define c(x) b ( # x )
    #define d(x) c ( x )
    #define e() d ( __FILE__ )
    e()
  )";
  constexpr char kTragedy[] = R"("\"\\\"\\\\\\\"test_input\\\\\\\"\\\"\"")";

  macro_map no_macros;
  llreader read("test_input", kTestCase, false);
  lexer lex(read, no_macros, error_constitutes_failure);

  token_t token = lex.get_token();
  ASSERT_THAT(token, HasType(TT_STRINGLITERAL));
  EXPECT_THAT(token.content.toString(), Eq(kTragedy));
}

}  // namespace jdi
