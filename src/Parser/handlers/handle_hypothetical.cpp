/**
 * @file handle_hypothetical.cpp
 * @brief Source implementing context_parser::handle_hypothetical.
 * 
 * @section License
 * 
 * Copyright (C) 2011-2012 Josh Ventura
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
#include <API/compile_settings.h>

namespace jdip {
  definition_hypothetical* handle_hypothetical(lexer *lex, definition_scope *scope, token_t& token, unsigned flags, error_handler *herr) {
    
    AST *a = new AST();
    if (a->parse_expression(token, lex, scope, precedence::scope, herr))
      { FATAL_RETURN(1); }
    
    definition_scope *temps;
    for (temps = scope; !(temps->flags & DEF_TEMPLATE); temps = temps->parent);
    if (!(temps->flags & DEF_TEMPLATE)) {
      token.report_error(herr, "Cannot infer type outside of template");
      return NULL;
    }
    definition_template *temp = (definition_template*)temps;
    
    // TODO: XXX: Should this use hypothetical at all? Or should it just use a template parameter?
    definition_hypothetical* h = new definition_hypothetical("<" + a->toString() + ">", scope, flags, a);
    temp->dependents.push_back(h);
    return h;
  }
}
