/**
 * @file read_next_token.cpp
 * @brief Source implementing the parser function to fetch the next token.
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

#include <Parser/bodies.h>
#include <API/context.h>
#include <General/parse_basics.h>
#include <General/debug_macros.h>
#include <Parser/parse_context.h>
#include <cstdio>
using namespace jdip;
using namespace jdi;

#define  c_file // Makes the code look prettier without making doxygen bitch
/**
  @section Implementation
  This implementation is relatively unremarkable, save for the bouts
  of dancing around llreader's structure. Normally, I would write such
  a function to play on null-terminated C strings. However, due to the
  variety of memory constructs an llreader may actually represent, that
  isn't an option here.
**/
token_t jdip::context_parser::read_next_token(lexer* lex, definition_scope *scope)
{
  token_t r = lex->get_token(pc->herr);
  if (r.type == TT_IDENTIFIER)
    return look_up_token(scope,string((const char*)r.extra.content.str,r.extra.content.len),r);
  return r;
}

token_t jdip::context_parser::look_up_token(definition_scope* scope, string name, token_t defalt)
{
  definition *def = scope->look_up(name);
  if (!def) return defalt;
  
  if (def->flags & DEF_TYPENAME) {
    defalt.extra.def = def;
    defalt.type = TT_DECLARATOR;
    return defalt;
  }
  
  return defalt;
}
