/**
 * @file  handle_templates.cpp
 * @brief Source implementing a function to handle a template<> declaration.
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
#include <API/AST.h>
#include <API/compile_settings.h>
#include <Parser/handlers/handle_function_impl.h>
#include <General/parse_basics.h>
#include <cstdio>

using namespace jdip;
#if FATAL_ERRORS
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
  token = read_next_token(scope);
  
  definition_template* temp = new definition_template("", scope, DEF_TEMPLATE | inherited_flags);
  
  for (;;) {
    string pname; // The name given to this parameter
    full_type ft(NULL); // The type of this parameter; default type or integer type
    
    definition_tempparam* dtn;
    if (token.type == TT_TYPENAME || token.type == TT_CLASS || token.type == TT_STRUCT) {
      token = lex->get_token(herr);
      if (token.type == TT_IDENTIFIER) {
        pname = token.content.toString();
        token = read_next_token(scope);
      }
      if (token.type == TT_OPERATOR) {
        if (token.content.len != 1 or *token.content.str != '=')
          token.report_error(herr, "Unexpected operator here; value must be denoted by '='");
        token = read_next_token(scope);
        full_type fts = read_fulltype(lex, token, temp, this, herr);
        ft.swap(fts);
        if (!ft.def) {
          token.report_error(herr,"Expected type name for default type to template parameter");
          return 1; // I can't think of a good way of recovering from this; we'll just end up trapped in this loop
        }
      }
      dtn = new definition_tempparam(pname, temp, ft, DEF_TYPENAME | DEF_TEMPPARAM);
    }
    else if (token.type == TT_DECFLAG || token.type == TT_DECLARATOR || token.type == TT_DECLTYPE) {
      full_type fts = read_fulltype(lex, token, scope, this, herr);
      ft.swap(fts);
      pname = ft.refs.name;
      AST *ast = NULL;
      if (token.type == TT_OPERATOR) {
        if (token.content.len != 1 or *token.content.str != '=')
          token.report_error(herr, "Unexpected operator here; value must be denoted by '='");
        token = read_next_token(scope);
        ast = new AST();
        ast->set_use_for_templates(true);
        ast->parse_expression(token, lex, scope, precedence::comma+1, herr);
      }
      dtn = new definition_tempparam(pname, temp, ft, ast, DEF_TEMPPARAM);
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
      sprintf(nname, "<templateParam%03u>", (unsigned)temp->params.size());
    }
    else
      temp->use_general(pname, dtn);
    if (token.type == TT_GREATERTHAN)
      break;
    if (token.type != TT_COMMA)
      token.report_errorf(herr, "Expected '>' or ',' before %s");
    token = read_next_token(scope);
  }
  
  token = read_next_token(temp);
  
  // ========================================================================================================================================
  // =====: Handle template class definitions :==============================================================================================
  // ========================================================================================================================================
  
  if (token.type == TT_CLASS || token.type == TT_STRUCT)
  {
    unsigned protection = token.type == TT_CLASS? DEF_PRIVATE : 0;
    token = read_next_token(scope);
    definition_class *tclass;
    
    if (token.type == TT_IDENTIFIER) {
      temp->name = token.content.toString();
      tclass = new definition_class(temp->name, temp, DEF_CLASS | DEF_TYPENAME);
      temp->def = tclass;
    
      scope->declare(temp->name, temp);
      
      regular_template_class:
      
      token = read_next_token(scope);
      if (token.type == TT_COLON) {
        if (handle_class_inheritance(temp, token, tclass, protection))
          return 1;
      }
      
      if (token.type != TT_LEFTBRACE) {
        if (token.type == TT_SEMICOLON) {
          tclass->flags |= DEF_INCOMPLETE;
          return 0;
        }
        token.report_errorf(herr, "Opening brace for class body expected before %s");
        return 1;
      }
      
      if (handle_scope(tclass, token, protection))
        FATAL_RETURN(1);
      
      if (token.type != TT_RIGHTBRACE) {
        token.report_errorf(herr, "Expected closing brace to class body before %s");
        FATAL_RETURN(1);
      }
      
      return 0;
    }
    else if (token.type == TT_DEFINITION) {
      if (not((token.def->flags & DEF_TEMPLATE) && (((definition_template*)token.def)->def->flags & DEF_CLASS))) {
        token.report_error(herr, "Expected class name for specialization; `" + token.def->name + "' does not name a template class");
        delete temp;
        return 1;
      }
      
      definition_template *basetemp = (definition_template*)token.def;
      
      token = read_next_token(temp);
      if (token.type != TT_LESSTHAN) {
        token.report_errorf(herr, "Expected opening triangle bracket for template definition before %s");
        delete temp;
        return 1;
      }
      
      arg_key argk(basetemp->params.size());
      definition_template::specialization *spec = new definition_template::specialization(basetemp->params.size(), temp->params.size(), temp);
      argk.mirror(basetemp);
      size_t args_given = 0;
      for (;;++args_given)
      {
        token = read_next_token(temp);
        if (token.type == TT_GREATERTHAN)
          break;
        if (token.type == TT_SEMICOLON || token.type == TT_LEFTBRACE) {
          token.report_errorf(herr, "Expected closing triangle bracket to template parameters before %s");
          break;
        }
        
        if (token.type == TT_COMMA) continue;
        
        if ((argk[args_given].type == arg_key::AKT_VALUE)
        and (token.type == TT_DEFINITION or token.type == TT_DECLARATOR) and token.def->flags & DEF_TEMPPARAM)
        {
          for (size_t i = 0; i < temp->params.size(); ++i) if (temp->params[i] == token.def) {
            if (~temp->params[i]->flags & DEF_TYPENAME) {
              spec->key.arg_inds[i][++spec->key.arg_inds[i][0]] = args_given;
              argk[args_given].val() = VT_DEPENDENT;
              token = read_next_token(temp);
              goto handled_argk; // I don't trust the optimizer enough to ugly up the code with a boolean for this. Suck it up.
            }
            else
              token.report_error(herr, "Type mismatch in passing parameter " + toString(args_given) + " to template specialization: real-valued parameter expected");
          }
        }
        
        if (read_template_parameter(argk, args_given, basetemp, lex, token, temp, this, herr))
          return 1;
        
        if (argk[args_given].type == arg_key::AKT_FULLTYPE) {
          definition *def = argk[args_given].ft().def;
          for (size_t i = 0; i < temp->params.size(); ++i) if (temp->params[i] == def) {
            spec->key.arg_inds[i][++spec->key.arg_inds[i][0]] = args_given;
            argk[args_given].ft().def = &arg_key::abstract;
          }
        }
        
        handled_argk:
        
        if (token.type == TT_GREATERTHAN)
          break;
        if (token.type != TT_COMMA) {
          token.report_errorf(herr, "Comma expected here before %s");
          break;
        }
      }
      if (check_read_template_parameters(argk, args_given, basetemp, token, herr))
        return 1;
      
      // cout << "Specialization key: " << argk.toString() << endl;
      definition_template::speclist &slist = basetemp->specializations[argk];
      
      for (definition_template::speclist::iterator it = slist.begin(); it != slist.end(); ++it)
        if ((*it)->key.same_as(spec->key))
        {
          delete spec;
          spec = *it;
          
          if (~spec->spec_temp->flags & DEF_INCOMPLETE) {
            token.report_error(herr, "Cannot specialize template: specialization by this type already exists.");
            delete temp;
            return 1;
          }
          
          spec->spec_temp->using_general.clear(); // XXX: Maybe replace this with something to specifically delete template parameters, just in case? Could anything else be used? I'm thinking not.
          for (definition_template::piterator pit = temp->params.begin(); pit != temp->params.end(); ++pit)
            spec->spec_temp->use_general((*pit)->name, *pit);
          
          delete temp;
          temp = spec->spec_temp;
          break;
        }
      slist.push_back(spec);
      
      temp->name = basetemp->name + "<" + argk.toString() + ">";
      tclass = new definition_class(temp->name, temp, DEF_CLASS | DEF_TYPENAME);
      temp->def = tclass;
      
      goto regular_template_class;
    }
    
    token.report_errorf(herr, "Expected class name here before %s");
    return 1;
  }
  
  // ========================================================================================================================================
  // =====: Handle template function definitions :===========================================================================================
  // ========================================================================================================================================
  
  if (token.type == TT_DECLARATOR || token.type == TT_DECFLAG || token.type == TT_DECLTYPE || token.type == TT_DEFINITION || token.type == TT_TYPENAME)
  {
    full_type funcrefs = read_fulltype(lex, token, temp, this, herr);
    if (!funcrefs.def) {
      token.report_error(herr, "Expected return type for template function at this point");
      delete temp;
      return 1;
    }
    
    if (funcrefs.refs.empty() || funcrefs.refs.top().type != ref_stack::RT_FUNCTION) {
      token.report_error(herr, "Definition in template must be a function");
      delete temp;
      return 1;
    }
    
    string funcname = funcrefs.refs.name;
    definition_function *func = NULL;
    definition *maybe = scope->find_local(funcname);
    if (maybe)
      if (!(maybe->flags & DEF_FUNCTION)) {
        token.report_error(herr, "Cannot redeclare " + maybe->kind() + " `" + funcname + "' as function in this scope");
        delete temp;
        return 1;
      }
      else func = (definition_function*)maybe;
    else {}
    
    definition_overload *tovr = new definition_overload(funcname, scope, funcrefs.def, funcrefs.refs, funcrefs.flags, DEF_FUNCTION);
    temp->def = tovr;
    
    if (!func)
      scope->declare(funcname, func = new definition_function(funcname, scope));
    func->overload(temp);
    
    if (token.type == TT_LEFTBRACE)
      tovr->implementation = (handle_function_implementation(lex, token, temp, herr));
    else if (token.type != TT_SEMICOLON)
      token.report_errorf(herr, "Expected template function body or semicolon before %s");
  }
  else if (token.type == TT_TEMPLATE) // Specialization
  {
    
  }
  else {
    token.report_errorf(herr, "Expected class or function declaration following template clause before %s");
    delete temp; return ERROR_CODE;
  }
  
  
  return 0;
}
