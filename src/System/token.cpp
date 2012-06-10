/**
 * @file token.cpp
 * @brief Source implementing token type constructors.
 * 
 * @section License
 * 
 * Copyright (C) 2011 Josh Ventura
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
using namespace jdip;

#ifdef DEBUG_MODE
namespace jdip {
  /// A debug listing of token descriptions by value ID
  const char* TOKEN_TYPE_NAME[TT_INVALID+1] = {
    "TT_DECLARATOR","TT_DECFLAG","TT_CLASS","TT_STRUCT","TT_ENUM","TT_UNION","TT_NAMESPACE","TT_EXTERN",
    "TT_ASM","TT_OPERATORKW","TT_SIZEOF","TT_DECLTYPE",
    "TT_IDENTIFIER",
    "TT_TEMPLATE","TT_TYPENAME",
    "TT_TYPEDEF","TT_USING",
    "TT_PUBLIC","TT_PRIVATE","TT_PROTECTED",
    "TT_COLON","TT_SCOPE",
    "TT_LEFTPARENTH","TT_RIGHTPARENTH","TT_LEFTBRACKET","TT_RIGHTBRACKET","TT_LEFTBRACE","TT_RIGHTBRACE","TT_LESSTHAN","TT_GREATERTHAN",
    "TT_TILDE","TT_ELLIPSIS","TT_OPERATOR","TT_COMMA","TT_SEMICOLON",
    "TT_STRINGLITERAL","TT_CHARLITERAL","TT_DECLITERAL","TT_HEXLITERAL","TT_OCTLITERAL",
    "TTM_CONCAT", "TTM_TOSTRING",
    "TT_ENDOFCODE","TT_INVALID"
    //,"error"
  };
}
#endif

token_t::token_t(): token_basics(type(TT_INVALID), file(""), linenum(), pos()) {}
token_t::token_t(token_basics(TOKEN_TYPE t, const char* fn, int l, int p)): token_basics(type(t), file(fn), linenum(l), pos(p)) {}
token_t::token_t(token_basics(TOKEN_TYPE t, const char* fn, int l, int p), const char* ct, int ctl): token_basics(type(t), file(fn), linenum(l), pos(p)), extra(ct, ctl) {}
token_t::token_t(token_basics(TOKEN_TYPE t, const char* fn, int l, int p), definition* def): token_basics(type(t), file(fn), linenum(l), pos(p)), extra(def) {}

token_t::extra_::extra_() {}
token_t::extra_::extra_(const char* ct, int ctl) { content.str = ct; content.len = ctl; }
token_t::extra_::extra_(definition* d): def(d) {}

void token_t::report_error(error_handler *herr, std::string error)
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

static struct token_info_c {
  const char *name[TT_INVALID+1];
  token_info_c() {
    for (int i = 0; i <= TT_INVALID; ++i)
      name[i] = "GLITCH: No name data afilliated with token. Oops.";
    TOKEN_TYPE a(TT_INVALID);
    switch (a) {
      default:
      case TT_INVALID: name[TT_INVALID] = "invalid token";
      case TT_DECLARATOR:
      case TT_DECFLAG: name[TT_DECFLAG] = "declarator";
      case TT_CLASS: name[TT_CLASS] = "`class' token";
      case TT_STRUCT: name[TT_STRUCT] = "`struct' token";
      case TT_ENUM: name[TT_ENUM] = "`enum' token";
      case TT_UNION: name[TT_UNION] = "`union' token";
      case TT_NAMESPACE: name[TT_NAMESPACE] = "`namespace' token";
      case TT_EXTERN: name[TT_EXTERN] = "`extern' token";
      case TT_ASM: name[TT_ASM] = "`asm' token";
      case TT_OPERATORKW: name[TT_OPERATORKW] = "`operator' token";
      case TT_SIZEOF: name[TT_OPERATORKW] = "`sizeof' token";
      case TT_DECLTYPE: name[TT_OPERATORKW] = "`decltype' token";
      case TT_IDENTIFIER: name[TT_IDENTIFIER] = "identifier";
      case TT_TEMPLATE: name[TT_TEMPLATE] = "`template' token";
      case TT_TYPENAME: name[TT_TYPENAME] = "`typename' token";
      case TT_TYPEDEF: name[TT_TYPEDEF] = "`typedef' token";
      case TT_USING: name[TT_USING] = "`using' token";
      case TT_PUBLIC: name[TT_PUBLIC] = "`public' token";
      case TT_PRIVATE: name[TT_PRIVATE] = "`private' token";
      case TT_PROTECTED: name[TT_PROTECTED] = "`protected' token";
      case TT_COLON: name[TT_COLON] = "`:' token";
      case TT_SCOPE: name[TT_SCOPE] = "`::' token";
      case TT_LEFTPARENTH: name[TT_LEFTPARENTH] = "'(' token";
      case TT_RIGHTPARENTH: name[TT_RIGHTPARENTH] = "')' token";
      case TT_LEFTBRACKET: name[TT_LEFTBRACKET] = "'[' token";
      case TT_RIGHTBRACKET: name[TT_RIGHTBRACKET] = "']' token";
      case TT_LEFTBRACE: name[TT_LEFTBRACE] = "'{' token";
      case TT_RIGHTBRACE: name[TT_RIGHTBRACE] = "'}' token";
      case TT_LESSTHAN: name[TT_LESSTHAN] = "'<' token";
      case TT_GREATERTHAN: name[TT_GREATERTHAN] = "'>' token";
      case TT_TILDE: name[TT_TILDE] = "'~' token";
      case TT_ELLIPSIS: name[TT_ELLIPSIS] = "`...' token";
      case TT_OPERATOR: name[TT_OPERATOR] = "operator";
      case TT_COMMA: name[TT_COMMA] = "',' token";
      case TT_SEMICOLON: name[TT_SEMICOLON] = "';' token";
      case TT_STRINGLITERAL: name[TT_STRINGLITERAL] = "string literal";
      case TT_CHARLITERAL: name[TT_CHARLITERAL] = "character literal";
      case TT_DECLITERAL: name[TT_DECLITERAL] = "decimal literal";
      case TT_HEXLITERAL: name[TT_HEXLITERAL] = "hexadecimal literal";
      case TT_OCTLITERAL: name[TT_OCTLITERAL] = "octal literal";
      case TTM_CONCAT:   name[TTM_CONCAT] = "`##' token";
      case TTM_TOSTRING: name[TTM_TOSTRING] = "`#' token";
      case TT_ENDOFCODE: name[TT_ENDOFCODE] = "end of code";
    }
  }
} token_info;

void token_t::report_errorf(error_handler *herr, std::string error)
{
  string fn; // Default values for non-existing info members
  int l = -1, p = -1;
  
  // Overwrite those which exist
  token_basics(p = -1,
    fn = (const char*)file,
    l = linenum,
    p = pos
  );
  
  size_t f = error.find("%s");
  while (f != string::npos) {
    error.replace(f,2,token_info.name[type]);
    f = error.find("%s");
  }
  
  herr->error(error, fn, l, p);
}
void token_t::report_warning(error_handler *herr, std::string error)
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
