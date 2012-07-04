/**
 * @file read_template_parameters.cpp
 * @brief Source implementing a function to handle class and structure declarations.
 * 
 * @section License
 * 
 * Copyright (C) 2012 Josh Ventura
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

#include <API/AST.h>
#include <Parser/bodies.h>
#include <API/compile_settings.h>

int jdip::read_template_parameters(definition_template::arg_key &argk, definition_template *temp, lexer *lex, token_t &token, definition_scope *scope, context_parser *cp, error_handler *herr)
{
  argk.mirror(temp);
  size_t args_given = 0;
  for (;;++args_given)
  {
    token = lex->get_token_in_scope(scope, herr);
    if (token.type == TT_GREATERTHAN)
      break;
    if (token.type == TT_SEMICOLON || token.type == TT_LEFTBRACE) {
      token.report_errorf(herr, "Expected closing triangle bracket to template parameters before %s");
      break;
    }
    
    if (token.type == TT_COMMA) continue;
    
    if (args_given < temp->params.size() and temp->params[args_given]->flags & DEF_TYPENAME) {
      full_type ft = read_type(lex, token, scope, cp, herr);
      if (ft.def) {
        definition_typed* const t = (definition_typed*)argk[args_given];
        t->type = ft.def;
        t->referencers.swap(ft.refs);
        t->modifiers = ft.flags;
      }
    } else {
      AST a;
      a.set_use_for_templates(true);
      a.parse_expression(token, lex, scope, herr);
      if (args_given < temp->params.size())
        ((definition_valued*)argk[args_given])->value_of = a.eval();
    }
    
    if (token.type == TT_GREATERTHAN)
      break;
    if (token.type != TT_COMMA) {
      token.report_errorf(herr, "Comma expected here before %s");
      break;
    }
  }
  if (args_given > temp->params.size()) {
      token.report_error(herr, "Too many template parameters provided to template `" + temp->name + "'");
      FATAL_RETURN(1);
  }
  int bad_params = 0;
  for (size_t i = 0; i < temp->params.size(); ++i)
    if (((argk[i]->flags & DEF_TYPENAME) and !((definition_typed*)argk[i])->type)
    or  ((argk[i]->flags & DEF_VALUED) and ((definition_valued*)argk[i])->value_of.type == VT_NONE))
      ++bad_params;
  if (bad_params) {
    token.report_error(herr, "Insufficient parameters to template `" + temp->name + "'");
    FATAL_RETURN(1);
  }
  return 0;
}
