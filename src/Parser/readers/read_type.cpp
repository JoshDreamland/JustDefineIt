/**
 * @file read_type.cpp
 * @brief Source implementing a utility funciton to read in a type.
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

/**
  @section Implementation
  This implementation is relatively unremarkable, save for the bouts
  of dancing around llreader's structure. Normally, I would write such
  a function to play on null-terminated C strings. However, due to the
  variety of memory constructs an llreader may actually represent, that
  isn't an option here.
**/
full_type jdip::context_parser::read_type(lexer *lex, token_t &token, definition_scope *scope)
{
  if (token.type != TT_DECLARATOR) {
    token = read_next_token(lex, scope);
    if (token.type != TT_DECLARATOR) {
      token.report_error(this,pc->herr,"Type name expected here");
      return full_type();
    }
  }
  long int rflags = 0;
  definition *rdef = token.extra.def;
  ref_stack rrefs;
  
  // Read any additional type info
  token = read_next_token(lex, scope);
  while (token.type == TT_DECLARATOR)
  {
    if (token.extra.def) {
      if (rdef) {
        token.report_error(this,pc->herr,"Two types named in declaration");
        return full_type();
      }
      rdef = token.extra.def;
    }
    token = read_next_token(lex, scope);
  }
  
  return full_type(rdef, rrefs, rflags);
}


