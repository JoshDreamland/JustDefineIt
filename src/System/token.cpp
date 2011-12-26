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
using namespace jdip;

#ifdef DEBUG_MODE
namespace jdip {
  /// A debug listing of token descriptions by value ID
  const char* TOKEN_TYPE_NAME[] = {
    "TT_DECLARATOR ///< Phrases like const, long, int, struct {}; anything that can be used as a type.",
    "TT_CLASS      ///< The `class' keyword.",
    "TT_STRUCT     ///< The `struct' keyword.",
    "TT_ENUM       ///< The `enum' keyword.",
    "TT_UNION      ///< The `union' keyword.",
    "TT_NAMESPACE  ///< The `namespace' keyword. ",
    
    "TT_IDENTIFIER, ///< A standard identifier.",
    
    "TT_TEMPLATE   ///< The `template' keyword, which should be followed by <...>",
    "TT_TYPENAME   ///< The `typename' keyword.",
    
    "TT_OPERATOR   ///< The `operator' keyword, not a random operator.",
    "TT_TYPEDEF    ///< The `typedef' keyword.",
    "TT_USING      ///< The `using' keyword.",
    
    "TT_PUBLIC     ///< The `public' keyword.",
    "TT_PRIVATE    ///< The `private' keyword.",
    "TT_PROTECTED  ///< The `protected' keyword.",
    
    "TT_COLON      ///< A simple colon, which should always mark a label.",
    "TT_SCOPE      ///< The scope accessor `::' symbol.",
    
    "TT_LEFTPARENTH   ///< A left parenthesis, `('.",
    "TT_RIGHTPARENTH  ///< A right parenthesis, `)'.",
    "TT_LEFTBRACKET   ///< A left bracket, `['.",
    "TT_RIGHTBRACKET  ///< A right bracket, `]'.",
    "TT_LEFTBRACE     ///< A left brace, `{'.",
    "TT_RIGHTBRACE    ///< A right brace, `}'.",
    "TT_LEFTTRIANGLE  ///< The less-than symbol `<'. Implies we're at a template.",
    "TT_RIGHTTRIANGLE ///< The greater-than symbol `>'. Implies we're ending a template.",
    
    "TT_DESTRUCTOR    ///< The tilde `~' symbol.",
    "TT_COMMA         ///< A comma, `,'. Separates items in lists.",
    "TT_SEMICOLON     ///< A semicolon, `;'. Separates statements and declarations.",
    "TT_EQUALS        ///< An equals sign, `='. Used for assigning default values and const values.",
    
    "TT_STRINGLITERAL ///< A string literal, such as \"hello, world!\"",
    "TT_DECLITERAL    ///< A decimal literal, such as 12345",
    "TT_HEXLITERAL    ///< A hexadecimal literal, such as 0xDEC0DED",
    "TT_OCTLITERAL    ///< An octal literal, such as 07654321.",
    
    "TT_ENDOFCODE ///< This token signifies that the code has reached its end.",
    "TT_INVALID ///< Invalid token; read failed."
  };
}
#endif

token_t::token_t(token_basics(TOKEN_TYPE t, const char* fn, int l, int p)): token_basics(type(t), file(fn), linenum(l), pos(p)) {}
token_t::token_t(token_basics(TOKEN_TYPE t, const char* fn, int l, int p), const char* ct, int ctl): token_basics(type(t), file(fn), linenum(l), pos(p)), extra(ct, ctl) {}
token_t::token_t(token_basics(TOKEN_TYPE t, const char* fn, int l, int p), definition* def): token_basics(type(t), file(fn), linenum(l), pos(p)), extra(def) {}

token_t::extra_::extra_() {}
token_t::extra_::extra_(const char* ct, int ctl) { content.str = ct; content.len = ctl; }
token_t::extra_::extra_(definition* d): def(d) {}

void token_t::report_error(parse_context *pc, std::string error) {
  // This is where we use the token_basics macro to only assign
  // those parse context members which exist.
  token_basics(
    pc->error = error,
    pc->err_file = (const char*)file,
    pc->err_line = linenum,
    pc->err_pos = pos
  );
}
