/**
 * @file read_operator_cast_type.cpp
 * @brief Source handling reading cast operator types.
 * 
 * This is a recurring problem thanks to templates, and a nontrivial problem
 * do to the way \c read_fulltype is set up. The other helper functions do not
 * stop reading at function parameters; in fact, there's no way to have them do
 * so without lookahead. However, that isn't ultimately a problem, as we can
 * simply read the function parameters in as part of the cast type, verify that
 * the outermost referencer on the stack is an empty set of parameters, and
 * finally just remove it from the final cast type.
 *
 * While the process isn't overly brilliant, it's sufficiently convoluted to
 * constitute its own black-box function.
 * 
 * @section License
 * 
 * Copyright (C) 2014 Josh Ventura
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

#include "System/lex_buffer.h"
#include "Parser/bodies.h"
#include "General/debug_macros.h"
#include "API/compile_settings.h"
#include "Storage/definition.h"

using namespace jdip;

full_type jdip::read_operatorkw_cast_type(lexer *lex, token_t &token, definition_scope *scope, context_parser *cp, error_handler *herr)
{
  token = lex->get_token_in_scope(scope);
  if (token.type != TT_DECLARATOR and token.type != TT_DECFLAG and token.type != TT_DECLTYPE) {
    token.report_errorf(herr, "Expected cast type to overload before %s");
    return NULL;
  }
  
  lex_buffer lb(lex);
  while (token.type != TT_LEFTPARENTH and token.type != TT_LEFTBRACE and token.type != TT_SEMICOLON and token.type != TT_ENDOFCODE)
    lb.push(token), token = lex->get_token_in_scope(scope);
  if (token.type != TT_LEFTPARENTH) {
    token.report_error(herr, "Expected function parmeters before %s");
    return NULL;
  }
  
  token.type = TT_ENDOFCODE; lb.push(token);
  token.type = TT_LEFTPARENTH;
  lb.reset(); token_t kick = lb.get_token(herr);
  full_type ft = read_fulltype(&lb, kick, scope, cp, herr);
  
  {
    ref_stack my_func_refs;
    read_referencers_post(my_func_refs, lex, token, scope, cp, herr);
    if (my_func_refs.empty() or my_func_refs.top().type != ref_stack::RT_FUNCTION) {
      token.report_error(herr, "Expected function parameters for operator overload");
      return NULL;
    }
    ft.refs.append_c(my_func_refs);
  }
  
  return ft;
}
