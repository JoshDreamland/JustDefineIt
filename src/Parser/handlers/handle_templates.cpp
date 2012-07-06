/**
 * @file  handle_templates.cpp
 * @brief Source implementing a function to handle a template<> declaration.
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
#include <API/AST.h>
#include <API/compile_settings.h>
#include <Parser/handlers/handle_function_impl.h>
#include <cstdio>

using namespace jdip;
#ifdef FATAL_ERRORS
#define ERROR_CODE 1
#else
#define ERROR_CODE 0
#endif

int context_parser::handle_template(definition_scope *scope, token_t& token, unsigned inherited_flags)
{
  token = read_next_token(scope);
  if (token.type != TT_LESSTHAN) {
    token.report_error(herr, "Expected opening triangle bracket following `template' token");
    return ERROR_CODE;
  }
  definition_template *temp = new definition_template("", scope, inherited_flags);
  definition_tempscope hijack("<template>", scope, DEF_TEMPLATE, temp);
  token = read_next_token(&hijack);
  for (;;) {
    string pname; // The name given to this parameter
    full_type ft(NULL); // The type of this parameter; default type or integer type
    
    definition_typed* dtn;
    if (token.type == TT_TYPENAME || token.type == TT_CLASS || token.type == TT_STRUCT) {
      token = lex->get_token(herr);
      if (token.type == TT_IDENTIFIER) {
        pname = token.content.toString();
        token = read_next_token(&hijack);
      }
      if (token.type == TT_OPERATOR) {
        if (token.content.len != 1 or *token.content.str != '=')
          token.report_error(herr, "Unexpected operator here; value must be denoted by '='");
        token = read_next_token(&hijack);
        full_type fts = read_fulltype(lex, token, &hijack, this, herr);
        ft.swap(fts);
        if (!ft.def) {
          token.report_error(herr,"Expected type name for default type to template parameter");
          return (delete temp, 1); // I can't think of a good way of recovering from this; we'll just end up trapped in this loop
        }
      }
      dtn = new definition_typed(pname, NULL, ft.def, ft.refs, ft.flags, DEF_TYPENAME | DEF_TYPED | DEF_TEMPLATE);
    }
    else if (token.type == TT_DECFLAG || token.type == TT_DECLARATOR || token.type == TT_DECLTYPE) {
      full_type fts = read_fulltype(lex, token, &hijack, this, herr);
      ft.swap(fts);
      pname = ft.refs.name;
      value val;
      if (token.type == TT_OPERATOR) {
        if (token.content.len != 1 or *token.content.str != '=')
          token.report_error(herr, "Unexpected operator here; value must be denoted by '='");
        AST a; a.parse_expression(lex, token, precedence::comma+1, herr);
        val = a.eval();
      }
      dtn = new definition_valued(pname, NULL, ft.def, ft.flags, DEF_VALUED | DEF_TYPED | DEF_TEMPLATE, val);
    }
    else {
      if (token.type == TT_GREATERTHAN) break;
      token.report_errorf(herr, "Expected '>' token here before %s");
      FATAL_RETURN((delete temp, 1));
      break;
    }
    
    temp->params.push_back(dtn);
    if (pname.empty()) {
      char nname[32];
      sprintf(nname, "<templateParam%u>", (unsigned)hijack.using_general.size());
      hijack.use_general(string(nname), dtn);
    }
    else
      hijack.use_general(pname, dtn);
    if (token.type == TT_GREATERTHAN)
      break;
    if (token.type != TT_COMMA)
      token.report_errorf(herr, "Expected '>' or ',' before %s");
    token = read_next_token(scope);
  }
  definition* nd = NULL;
  token = read_next_token(scope);
  
  if (token.type == TT_CLASS || token.type == TT_STRUCT) {
    if (handle_declarators(&hijack,token,inherited_flags | DEF_TEMPLATE, nd)) {
      return (delete temp, 1);
    }
    if (nd) {
      token.report_error(herr, "Cannot declare `" + nd->name + "' as abstract template type");
      FATAL_RETURN((delete temp, 1));
    }
  } else if (token.type == TT_DECLARATOR || token.type == TT_DECFLAG || token.type == TT_DECLTYPE) {
    if (handle_declarators(&hijack,token,inherited_flags | DEF_TEMPLATE, nd) or !nd)
      return (delete temp, 1);
  } else {
    token.report_error(herr, "Expected class or function declaration following template clause");
    delete temp; return ERROR_CODE;
  }
  
  if (hijack.members.empty()) {
    if (hijack.referenced)
      return 0;
    token.report_error(herr, "Template declaration doesn't declare anything");
    delete temp; return ERROR_CODE;
  }
  
  if (hijack.members.size() != 1) {
    char buf[96]; sprintf(buf, "Too many declarations for template; expected one, given %d", (int)hijack.members.size());
    token.report_error(herr, buf);
  }
  
  definition_scope::defiter a = hijack.members.begin();
  temp->name = a->second->name;
  temp->def = a->second;
  hijack.members.erase(a);
  
  pair<definition_scope::defiter,bool> i = scope->members.insert(pair<string,definition*>(temp->name,temp));
  
  if (!i.second) {
    definition * const redec = i.first->second;
    // Specialization
    if (temp->def->flags & DEF_FUNCTION) {
      if (redec->flags & DEF_TEMPLATE) {
        if (token.type == TT_LESSTHAN) {
          
        }
        token.report_error(herr, "Unimplemented: template function specialization");
        delete temp;
      }
      else {
        token.report_error(herr, "Cannot redeclare `" + temp->name + "' as template");
        delete temp; return ERROR_CODE;
      }
    }
    else {
      token.report_error(herr, "Cannot redeclare `" + temp->name + "' as template");
      delete temp; return ERROR_CODE;
    }
  }
  return 0;
}
