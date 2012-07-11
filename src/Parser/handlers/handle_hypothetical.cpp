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
  definition* context_parser::handle_hypothetical(definition_scope *scope, token_t& token, unsigned flags) {
    // Verify that we're in a template<> statement.
    definition_scope* temps;
    for (temps = scope; temps and not (temps->flags & DEF_TEMPLATE); temps = temps->parent);
    if (!temps) {
      token.report_errorf(herr, "Invalid use of `typename' keyword: must be in a template");
      return NULL;
    }
    
    // We are in a template<> declaration. Insert our hypothetical 
    definition_template* temp = (definition_template*)((definition_tempscope*)temps)->source;
    
    full_type ft = read_type(lex, token, scope, this, herr);
    if (!ft.def) { token.report_errorf(herr, "Type expected here before %s"); return NULL; }
    
    definition_template::dependent_qualification dq;
    dq.depends = ft.def;
    
    while (token.type  == TT_SCOPE) {
      token = read_next_token(scope);
      if (token.type != TT_IDENTIFIER and token.type != TT_DEFINITION and token.type != TT_DECLARATOR) {
        token.report_errorf(herr, "Expected identifier to qualify before %s");
        FATAL_RETURN(NULL);
        break;
      }
      dq.path.push_back(token.content.toString());
      token = read_next_token(scope);
    }
    
    if (!dq.path.size())
      return dq.depends;
    
    pair<definition_template::depiter, bool> ins = temp->dependents.insert(pair<definition_template::dependent_qualification, definition*>(dq, NULL));
    if (ins.second)
      return ins.first->second = new definition_hypothetical(dq.path[dq.path.size()-1], (definition_scope*)dq.depends, flags);
    return ins.first->second;
  }
}
