/**
 * @file read_qualified_definition.cpp
 * @brief Source implementing the parser function to read an entire qualified-id.
 * 
 * @section License
 * 
 * Copyright (C) 2011-2013 Josh Ventura
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
#include <iostream>

using namespace jdip;
definition* jdip::read_qualified_definition(lexer *lex, definition_scope* scope, token_t &token, context_parser *cp, error_handler *herr)
{
  definition *res = NULL;
  if (token.type == TT_SCOPE) {
    token = lex->get_token_in_scope(scope);
    res = token.def = cp->get_global();
  }
  if (token.type == TT_IDENTIFIER) {
    token.report_error(herr, "Expeceted qualified-id here; `" + token.content.toString() + "' is not declared");
    return NULL;
  }
  #ifdef DEBUG_MODE
  if (!token.def) {
    std::cerr << "This function was invoked improperly." << std::endl;
    abort();
  }
  #endif
  for (;;) {
    #ifdef DEBUG_MODE
    if (!token.def) {
      std::cerr << "Something in the program went wrong." << std::endl;
      abort();
    }
    #endif
    if (token.def->flags & DEF_TEMPLATE)
    {
      res = token.def;
      definition_template* dt = (definition_template*)res;
      if (dt->def && dt->def->flags & DEF_CLASS)
      {
        token = lex->get_token_in_scope(scope, herr);
        if (token.type == TT_LESSTHAN)
        {
          arg_key k(dt->params.size());
          if (read_template_parameters(k, dt, lex, token, scope, cp, herr))
            return FATAL_TERNARY(NULL,res);
          if (k.is_dependent())
            res = handle_dependent_tempinst(scope, token, dt, k, 0, herr);
          else
            res =  dt->instantiate(k, herr);
          if (token.type != TT_GREATERTHAN)
            token.report_errorf(herr, "Expected closing triangle bracket before %s");
          token = lex->get_token_in_scope(scope,herr);
        }
      }
      else {
        token.report_error(herr, "Template `" + token.def->name + "' cannot be used as a type");
        //cerr << token.def->toString();
        return FATAL_TERNARY(NULL,res);
      }
    }
    else if (token.def->flags & DEF_SCOPE)
    {
      definition_scope* as = (definition_scope*)token.def;
      token = lex->get_token_in_scope(scope, herr);
      if (token.type != TT_SCOPE)
        break;
      token = lex->get_token_in_scope(as, herr);
      if (token.type != TT_DEFINITION and token.type != TT_DECLARATOR) {
        if (token.type == TT_OPERATORKW) {
          res = token.def = as->look_up(read_operatorkw_name(lex, token, scope, herr));
          if (!token.def)
            return FATAL_TERNARY(NULL,res);
          if (token.type != TT_SCOPE)
            break;
          continue;
        }
        else {
          token.report_errorf(herr, "Expected type or qualified-id before %s");
          return FATAL_TERNARY(NULL,res);
        }
      }
      res = token.def;
      continue;
    }
    else {
      token = lex->get_token_in_scope(scope, herr);
      break;
    }
    
    if (token.type == TT_SCOPE) {
      #ifdef DEBUG_MODE
        if (!res) { token.report_error(herr, "Accessing NULL scope..."); return NULL; }
        if (!(res->flags & DEF_SCOPE)) { token.report_error(herr, "Accessing non-scope object " + res->name + "..."); return NULL; }
      #endif
      token = lex->get_token(herr);
      if (token.type != TT_IDENTIFIER) {
        token.report_errorf(herr, "Expected variable name following `::' before %s");
        return NULL;
      }
      else {
        // This loop checks token.def for things; we have to set it here.
        token.def = ((definition_scope*)res)->get_local(token.content.toString());
        if (!token.def) {
          token.report_error(herr, "Scope `" + res->name + "' does not contain `" + token.content.toString() + "'");
          return NULL;
        }
        res = token.def;
      }
    }
    else break;
  }
  return res;
}
