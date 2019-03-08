/**
 * @file token.cpp
 * @brief Source implementing token type constructors.
 * 
 * @section License
 * 
 * Copyright (C) 2011-2014 Josh Ventura
 * This file is part of JustDefineIt.
 * 
 * JustDefineIt is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3 of the License, or (at your option) any later version.
 * 
 * JustDefineIt is distributed in the hope that it will be useful, but WITHOUT ANY 
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * JustDefineIt. If not, see <http://www.gnu.org/licenses/>.
**/

#include "token.h"
#include <API/context.h>
using namespace jdi;

#ifdef DEBUG_MODE
  #define cdebuginit(x,y) ,x(y)
#else
  #define cdebuginit(x,y)
#endif
  
token_t::token_t(): token_basics(type(TT_INVALID), file(""), linenum(), pos()) {}
token_t::token_t(token_basics(TOKEN_TYPE t, const char* fn, int l, int p)): token_basics(type(t), file(fn), linenum(l), pos(p)) cdebuginit(def,NULL) {}
token_t::token_t(token_basics(TOKEN_TYPE t, const char* fn, int l, int p), const char* ct, int ctl): token_basics(type(t), file(fn), linenum(l), pos(p)), content(ct, ctl) cdebuginit(def,NULL) {}
token_t::token_t(token_basics(TOKEN_TYPE t, const char* fn, int l, int p), definition* d): token_basics(type(t), file(fn), linenum(l), pos(p)), def(d) {}

void token_t::report_error(error_handler *herr, std::string error) const
{
  string fn; // Default values for non-existing info members
  int l = -1, p = -1;
  
  // Overwrite those which exist
  token_basics(p = -1,
    fn = (const char*)file,
    l = linenum,
    p = pos
  );
  
  herr->error(error, fn, l, p);
}

#include <cstdio>
static struct token_info_c {
  string name[TT_INVALID+1];
  token_info_c() {
    for (int i = 0; i <= TT_INVALID; ++i) {
      char buf[128];
      sprintf(buf, "GLITCH: No name data afilliated with token %d. Oops.", i);
      name[i] = buf;
    }
    TOKEN_TYPE a(TT_INVALID);
    switch (a) {
      default:
      case TT_INVALID: name[TT_INVALID] = "invalid token"; // Fallthrough
      
      case TT_DECLARATOR:name[TT_DECLARATOR] = "declarator"; // Fallthrough
      case TT_DECFLAG: name[TT_DECFLAG] = "declarator"; // Fallthrough
      case TT_CLASS: name[TT_CLASS] = "`class' token"; // Fallthrough
      case TT_STRUCT: name[TT_STRUCT] = "`struct' token"; // Fallthrough
      case TT_ENUM: name[TT_ENUM] = "`enum' token"; // Fallthrough
      case TT_UNION: name[TT_UNION] = "`union' token"; // Fallthrough
      case TT_NAMESPACE: name[TT_NAMESPACE] = "`namespace' token"; // Fallthrough
      case TT_EXTERN: name[TT_EXTERN] = "`extern' token"; // Fallthrough
      case TT_INLINE: name[TT_INLINE] = "`inline' token"; // Fallthrough
      
      case TT_ASM: name[TT_ASM] = "`asm' token"; // Fallthrough
      case TT_OPERATORKW: name[TT_OPERATORKW] = "`operator' token"; // Fallthrough
      case TT_SIZEOF: name[TT_SIZEOF] = "`sizeof' token"; // Fallthrough
      case TT_ISEMPTY: name[TT_ISEMPTY] = "`is_empty' token"; // Fallthrough
      case TT_DECLTYPE: name[TT_DECLTYPE] = "`decltype' token"; // Fallthrough
      case TT_TYPEID: name[TT_TYPEID] = "`typeid' token"; // Fallthrough
      
      case TT_ALIGNAS: name[TT_ALIGNAS] = "`alignas' token"; // Fallthrough
      case TT_ALIGNOF: name[TT_ALIGNOF] = "`alignof' token"; // Fallthrough
      case TT_AUTO: name[TT_AUTO] = "`autp' token"; // Fallthrough
      case TT_CONSTEXPR: name[TT_CONSTEXPR] = "`constexpr' token"; // Fallthrough
      case TT_NOEXCEPT: name[TT_NOEXCEPT] = "`noexcept' token"; // Fallthrough
      case TT_STATIC_ASSERT: name[TT_STATIC_ASSERT] = "`static_assert' token"; // Fallthrough
      
      case TT_IDENTIFIER: name[TT_IDENTIFIER] = "identifier (\"%s\")"; // Fallthrough
      case TT_DEFINITION: name[TT_DEFINITION] = "identifier (\"%s\")"; // Fallthrough
      
      case TT_TEMPLATE: name[TT_TEMPLATE] = "`template' token"; // Fallthrough
      case TT_TYPENAME: name[TT_TYPENAME] = "`typename' token"; // Fallthrough
      
      case TT_TYPEDEF: name[TT_TYPEDEF] = "`typedef' token"; // Fallthrough
      case TT_USING: name[TT_USING] = "`using' token"; // Fallthrough
      
      case TT_PUBLIC: name[TT_PUBLIC] = "`public' token"; // Fallthrough
      case TT_PRIVATE: name[TT_PRIVATE] = "`private' token"; // Fallthrough
      case TT_PROTECTED: name[TT_PROTECTED] = "`protected' token"; // Fallthrough
      case TT_FRIEND: name[TT_FRIEND] = "`friend' token"; // Fallthrough
      
      case TT_COLON: name[TT_COLON] = "`:' token"; // Fallthrough
      case TT_SCOPE: name[TT_SCOPE] = "`::' token"; // Fallthrough
      case TT_MEMBER: name[TT_MEMBER] = "member specifier (class::*)"; // Fallthrough
      
      case TT_LEFTPARENTH: name[TT_LEFTPARENTH] = "'(' token"; // Fallthrough
      case TT_RIGHTPARENTH: name[TT_RIGHTPARENTH] = "')' token"; // Fallthrough
      case TT_LEFTBRACKET: name[TT_LEFTBRACKET] = "'[' token"; // Fallthrough
      case TT_RIGHTBRACKET: name[TT_RIGHTBRACKET] = "']' token"; // Fallthrough
      case TT_LEFTBRACE: name[TT_LEFTBRACE] = "'{' token"; // Fallthrough
      case TT_RIGHTBRACE: name[TT_RIGHTBRACE] = "'}' token"; // Fallthrough
      case TT_LESSTHAN: name[TT_LESSTHAN] = "'<' token"; // Fallthrough
      case TT_GREATERTHAN: name[TT_GREATERTHAN] = "'>' token"; // Fallthrough
      
      case TT_PLUS: name[TT_PLUS] = "'+' operator"; // Fallthrough
      case TT_MINUS: name[TT_MINUS] = "'-' operator"; // Fallthrough
      case TT_STAR: name[TT_STAR] = "'*' operator"; // Fallthrough
      case TT_SLASH: name[TT_SLASH] = "'/' operator"; // Fallthrough
      case TT_MODULO: name[TT_MODULO] = "'%' operator"; // Fallthrough
      case TT_EQUAL_TO: name[TT_EQUAL_TO] = "`==' operator"; // Fallthrough
      case TT_NOT_EQUAL_TO: name[TT_NOT_EQUAL_TO] = "`!=' operator"; // Fallthrough
      case TT_LESS_EQUAL: name[TT_LESS_EQUAL] = "`<=' operator"; // Fallthrough
      case TT_GREATER_EQUAL: name[TT_GREATER_EQUAL] = "`>=' operator"; // Fallthrough
      case TT_AMPERSAND: name[TT_AMPERSAND] = "'&' operator"; // Fallthrough
      case TT_AMPERSANDS: name[TT_AMPERSANDS] = "`&&' operator"; // Fallthrough
      case TT_PIPE: name[TT_PIPE] = "'|' operator"; // Fallthrough
      case TT_PIPES: name[TT_PIPES] = "`||' operator"; // Fallthrough
      case TT_CARET: name[TT_CARET] = "'^' operator"; // Fallthrough
      case TT_INCREMENT: name[TT_INCREMENT] = "increment (`++') operator"; // Fallthrough
      case TT_DECREMENT: name[TT_DECREMENT] = "decrement (`--') operator"; // Fallthrough
      case TT_ARROW: name[TT_ARROW] = "`->' operator"; // Fallthrough
      case TT_DOT: name[TT_DOT] = "`.' operator"; // Fallthrough
      case TT_ARROW_STAR: name[TT_ARROW_STAR] = "`->*' operator"; // Fallthrough
      case TT_DOT_STAR: name[TT_DOT_STAR] = "`.*' operator"; // Fallthrough
      case TT_QUESTIONMARK: name[TT_QUESTIONMARK] = "'?' operator"; // Fallthrough
      case TT_EQUAL: name[TT_EQUAL] = "`=' operator"; // Fallthrough
      case TT_ADD_ASSIGN: name[TT_ADD_ASSIGN] = "`+=' operator"; // Fallthrough
      case TT_SUBTRACT_ASSIGN: name[TT_SUBTRACT_ASSIGN] = "`-=' operator"; // Fallthrough
      case TT_MULTIPLY_ASSIGN: name[TT_MULTIPLY_ASSIGN] = "`*=' operator"; // Fallthrough
      case TT_DIVIDE_ASSIGN: name[TT_DIVIDE_ASSIGN] = "`/=' operator"; // Fallthrough
      case TT_MODULO_ASSIGN: name[TT_MODULO_ASSIGN] = "`%=' operator"; // Fallthrough
      case TT_LSHIFT_ASSIGN: name[TT_LSHIFT_ASSIGN] = "`<<=' operator"; // Fallthrough
      case TT_RSHIFT_ASSIGN: name[TT_RSHIFT_ASSIGN] = "`>>=' operator"; // Fallthrough
      case TT_AND_ASSIGN: name[TT_AND_ASSIGN] = "`&=' operator"; // Fallthrough
      case TT_OR_ASSIGN: name[TT_OR_ASSIGN] = "`|=' operator"; // Fallthrough
      case TT_XOR_ASSIGN: name[TT_XOR_ASSIGN] = "`^=' operator"; // Fallthrough
      case TT_NEGATE_ASSIGN: name[TT_NEGATE_ASSIGN] = "`~=' operator"; // Fallthrough
      case TT_LSHIFT: name[TT_LSHIFT] = "`<<' operator"; // Fallthrough
      case TT_RSHIFT: name[TT_RSHIFT] = "`>>' operator"; // Fallthrough
      case TT_NOT: name[TT_NOT] = "`!' operator"; // Fallthrough
      case TT_TILDE: name[TT_TILDE] = "'~' token"; // Fallthrough
      case TT_ELLIPSIS: name[TT_ELLIPSIS] = "`...' token"; // Fallthrough
      
      case TT_COMMA: name[TT_COMMA] = "',' token"; // Fallthrough
      case TT_SEMICOLON: name[TT_SEMICOLON] = "';' token"; // Fallthrough
      
      case TT_STRINGLITERAL: name[TT_STRINGLITERAL] = "string literal"; // Fallthrough
      case TT_CHARLITERAL: name[TT_CHARLITERAL] = "character literal"; // Fallthrough
      case TT_DECLITERAL: name[TT_DECLITERAL] = "decimal literal"; // Fallthrough
      case TT_HEXLITERAL: name[TT_HEXLITERAL] = "hexadecimal literal"; // Fallthrough
      case TT_OCTLITERAL: name[TT_OCTLITERAL] = "octal literal"; // Fallthrough
      case TT_BINLITERAL: name[TT_BINLITERAL] = "binary literal"; // Fallthrough
      
      case TTM_CONCAT:   name[TTM_CONCAT] = "`##' token"; // Fallthrough
      case TTM_TOSTRING: name[TTM_TOSTRING] = "`#' token"; // Fallthrough
      case TTM_COMMENT:  name[TTM_COMMENT] = "...comment(?)"; // Fallthrough
      case TTM_NEWLINE:  name[TTM_NEWLINE] = "...newline(?)"; // Fallthrough
      
      case TT_NEW: name[TT_NEW] = "`new' keyword"; // Fallthrough
      case TT_DELETE: name[TT_DELETE] = "`delete' keyword"; // Fallthrough
      
      case TT_CONST_CAST: name[TT_CONST_CAST] = "`const_cast' keyword"; // Fallthrough
      case TT_STATIC_CAST: name[TT_STATIC_CAST] = "`static_cast' keyword"; // Fallthrough
      case TT_DYNAMIC_CAST: name[TT_DYNAMIC_CAST] = "`dynamic_cast' keyword"; // Fallthrough
      case TT_REINTERPRET_CAST: name[TT_REINTERPRET_CAST] = "`reinterpret_cast' keyword"; // Fallthrough
      
      case TT_ENDOFCODE: name[TT_ENDOFCODE] = "end of code";
      #define handle_user_token(tname, desc) case tname: name[tname] = desc;
      #include <User/token_cases.h>
    }
  }
} token_info;

#include <cstdio>
void token_t::report_errorf(error_handler *herr, std::string error) const
{
  string fn; // Default values for non-existing info members
  int l = -1, p = -1;
  
  // Overwrite those which exist
  token_basics(p = -1,
    fn = (const char*)file,
    l = linenum,
    p = pos
  );
  
  string str = to_string();
  size_t f = error.find("%s");
  while (f != string::npos) {
    error.replace(f, 2, str);
    f = error.find("%s");
  }
  
  #ifdef DEBUG_MODE
    if (herr)
  #endif
  herr->error(error, fn, l, p);
  #ifdef DEBUG_MODE
    else {
      perror("NULL ERROR HANDLER PASSED TO REPORT METHOD\n");
      perror((error + "\n").c_str());
    }
  #endif
}
std::string token_t::to_string() const {
  size_t f;
  string str = token_info.name[type];
  f = str.find("%s");
  while (f != string::npos) {
    str.replace(f, 2, string((const char*) content.str, content.len));
    f = str.find("%s");
  }
  return str;
}
void token_t::report_warning(error_handler *herr, std::string error) const
{
  string fn; // Default values for non-existing info members
  int l = -1, p = -1;
  
  // Overwrite those which exist
  token_basics(p = -1,
    fn = (const char*)file,
    l = linenum,
    p = pos
  );
  
  herr->warning(error, fn, l, p);
}

std::string token_t::get_name(TOKEN_TYPE tt) {
  size_t f;
  string str = token_info.name[tt];
  f = str.find("%s");
  while (f != string::npos) {
    str.replace(f, 2, "(content)");
    f = str.find("%s");
  }
  return str;
}
