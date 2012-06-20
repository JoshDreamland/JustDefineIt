/**
 * @file  handle_scope.cpp
 * @brief Source implementing a massive delegator which populates a scope.
 * 
 * This file does a huge amount of work.
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
 * 
**/

#include <Parser/bodies.h>
#include <API/AST.h>
#include <API/compile_settings.h>
#include <Parser/handlers/handle_function_implementation.h>
#include <cstdio>

int jdip::context_parser::handle_scope(definition_scope *scope, token_t& token, unsigned inherited_flags)
{
  definition* decl;
  token = read_next_token(scope);
  for (;;)
  {
    switch (token.type)
    {
      case TT_DECFLAG: case TT_DECLTYPE: case TT_DECLARATOR:
      case TT_CLASS: case TT_STRUCT: case TT_ENUM: case TT_UNION: case TT_TILDE:
          decl = NULL;
          handle_declarator_block:
          if (handle_declarators(scope, token, inherited_flags, decl))
            return 1;
          if (token.type != TT_SEMICOLON) {
            if (token.type == TT_LEFTBRACE || token.type == TT_ASM) {
              if (!(decl and decl->flags & DEF_FUNCTION)) {
                token.report_error(herr, "Unexpected opening brace here; declaration is not a function");
                return 1;
              } else {
                ((definition_function*)decl)->implementation = handle_function_implementation(lex,token,scope,herr);
                if (token.type != TT_RIGHTBRACE && token.type != TT_SEMICOLON) {
                  token.report_error(herr, "Expected closing symbol to function");
                  continue;
                }
              }
            }
            else {
              token.report_errorf(herr, "Expected semicolon before %s following declaration");
              #if FATAL_ERRORS
                return 1;
              #else
                while (token.type != TT_SEMICOLON && token.type != TT_LEFTBRACE && token.type != TT_RIGHTBRACE && token.type != TT_ENDOFCODE)
                  token = read_next_token(scope);
              #endif
            }
          }
        break;
      
      case TT_EXTERN:
          token = read_next_token(scope);
          if (token.type == TT_STRINGLITERAL) {
            token = read_next_token(scope);
            if (token.type == TT_LEFTBRACE) {
              FATAL_RETURN_IF(handle_scope(scope, token, inherited_flags), 1);
              if (token.type != TT_RIGHTBRACE) {
                token.report_error(herr, "Expected closing brace to extern block");
                FATAL_RETURN(1);
              }
              break;
            }
          }
          goto handle_declarator_block;
        break;
      
      case TT_COMMA:
          token.report_error(herr, "Unexpected comma at this point.");
        return 1;
      
      case TT_SEMICOLON:
          /* Printing a warning here is advisable but unnecessary. */
        break;
      
      case TT_NAMESPACE: if (handle_namespace(scope,token)) return 1; break;
      case TT_LEFTPARENTH: {
          token.report_error(herr, "Stray opening parenthesis.");
          #if FATAL_ERRORS
            return 1;
          #else
            int bc = 1;
            while (bc) {
              token = read_next_token(scope);
              bc += token.type == TT_LEFTPARENTH;
              bc -= token.type == TT_RIGHTPARENTH;
            }
          #endif
        } break;
      case TT_RIGHTPARENTH: token.report_error(herr, "Stray closing parenthesis."); return 1;
      case TT_LEFTBRACKET:  token.report_error(herr, "Stray opening bracket."); return 1;
      case TT_RIGHTBRACKET: token.report_error(herr, "Stray closing bracket."); return 1;
      case TT_RIGHTBRACE:   return 0;
      case TT_LEFTBRACE: {
          token.report_error(herr, "Expected scope declaration before opening brace.");
          #if FATAL_ERRORS
            return 1;
          #else
            int bc = 1;
            while (bc) {
              token = read_next_token(scope);
              bc += token.type == TT_LEFTBRACE;
              bc -= token.type == TT_RIGHTBRACE;
            }
          #endif
        } break;
      
      case TT_TYPEDEF:
        token = read_next_token(scope);
        if (handle_declarators(scope,token,inherited_flags | DEF_TYPENAME)) FATAL_RETURN(1); break;
      
      case TT_PUBLIC:
        if (scope->flags & TT_CLASS) { inherited_flags &= ~(DEF_PRIVATE | DEF_PROTECTED); }
        else token.report_error(herr, "Unexpected `public' token outside class scope.");
        if ((token = read_next_token(scope)).type != TT_COLON)
          token.report_error(herr, "Colon expected following `public' token"); break;
      case TT_PRIVATE:
        if (scope->flags & TT_CLASS) { inherited_flags &= ~(DEF_PRIVATE | DEF_PROTECTED); inherited_flags |= DEF_PRIVATE; }
        else token.report_error(herr, "Unexpected `private' token outside class scope."); break;
        if ((token = read_next_token(scope)).type != TT_COLON)
          token.report_error(herr, "Colon expected following `private' token"); break;
      case TT_PROTECTED:
        if (scope->flags & TT_CLASS) { inherited_flags &= ~(DEF_PRIVATE | DEF_PROTECTED); inherited_flags |= DEF_PROTECTED; }
        else token.report_error(herr, "Unexpected `protected' token outside class scope.");
        if ((token = read_next_token(scope)).type != TT_COLON)
          token.report_error(herr, "Colon expected following `protected' token"); break;
      
      case TT_USING:
          token = read_next_token(scope);
          if (token.type == TT_NAMESPACE) {
            token = lex->get_token(herr);
            if (token.type == TT_IDENTIFIER) {
              definition* d = scope->look_up(token.content.toString());
              if (d->flags & DEF_NAMESPACE)
                scope->use_namespace((definition_scope*)d);
              else
                token.report_error(herr, "Expected namespace name following `namespace' token");
              token = read_next_token(scope);
            }
            else
              token.report_error(herr, "Expected namespace name following `namespace' token");
          }
          else {
              // TODO: Implement me
          }
        break;
      
      case TT_SCOPE:
          token = read_next_token((definition_scope*)token.def);
        continue;
      case TT_DEFINITION: {
        if (token.def->flags & DEF_NAMESPACE) {
          definition_scope* dscope = (definition_scope*)token.def;
          token = read_next_token(scope);
          if (token.type == TT_SCOPE) {
            token = read_next_token(dscope);
            continue;
          }
          token.report_errorf(herr, "Expected `::' here to access namespace members");
          FATAL_RETURN(1); break;
        }
      }
      case TT_IDENTIFIER:
          token.report_error(herr, "Unexpected identifier in this scope; `" + string(token.content.toString()) + "' does not name a type");
        break;
      
      case TT_TEMPLATE: {
          token = read_next_token(scope);
          if (token.type != TT_LESSTHAN) {
            token.report_error(herr, "Expected opening triangle bracket following `template' token");
            FATAL_RETURN(1); break;
          }
          definition_scope hijack("<template>",scope, DEF_NAMESPACE | DEF_TEMPLATE);
          definition_template *temp = new definition_template();
          token = read_next_token(scope);
          for (;;) {
            string pname; // The name given to this parameter
            full_type ft(NULL); // The type of this parameter; default type or integer type
            
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
                if (not(token.type == TT_DECFLAG || token.type == TT_DECLARATOR || token.type == TT_DECLTYPE)) {
                  token.report_error(herr, "Expected type name for default type to template parameter");
                  return (delete temp, 1); // I can't think of a good way of recovering from this; we'll just end up trapped in this loop
                }
                ft = read_type(lex, token, scope, this, herr);
                
              }
            }
            else if (token.type == TT_DECFLAG || token.type == TT_DECLARATOR || token.type == TT_DECLTYPE) {
              ft = read_type(lex, token, scope, this, herr);
              pname = ft.refs.name;
              if (token.type == TT_OPERATOR) {
                if (token.content.len != 1 or *token.content.str != '=')
                  token.report_error(herr, "Unexpected operator here; value must be denoted by '='");
                AST a; a.parse_expression(lex, token, herr);
              }
            };
            definition_typed* const dtn = new definition_typed(pname, NULL, ft.def, ft.refs, ft.flags, DEF_TYPENAME | DEF_TYPED);
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
          }
          definition* nd;
          token = read_next_token(scope);
          if (token.type == TT_CLASS || token.type == TT_STRUCT) {
            if (!(nd = handle_class(&hijack, token, DEF_TEMPLATE)))
              return (delete temp, 1);
          } else if (token.type == TT_DECLARATOR || token.type == TT_DECFLAG || token.type == TT_DECLTYPE) {
            if (!handle_declarators(&hijack,token,inherited_flags | DEF_TEMPLATE, nd) or !nd)
              return (delete temp, 1);
          } else {
            token.report_error(herr, "Expected class or function declaration following template clause");
            delete temp; break;
          }
          
          if (!hijack.members.size()) {
            token.report_error(herr, "Template declaration doesn't declare anything");
            delete temp; break;
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
            token.report_error(herr, "Redeclaration of `" + temp->name + "' as template (TODO: Function overloading)");
            delete temp;
          }
        } break;
      
      case TT_OPERATORKW:
        //break;
      
      case TT_TYPENAME: case TT_ASM: case TT_SIZEOF:
      case TT_OPERATOR: case TT_ELLIPSIS: case TT_LESSTHAN: case TT_GREATERTHAN: case TT_COLON:
      case TT_DECLITERAL: case TT_HEXLITERAL: case TT_OCTLITERAL: case TT_STRINGLITERAL: case TT_CHARLITERAL:
      case TTM_CONCAT: case TTM_TOSTRING: case TT_INVALID:
      default:
        token.report_errorf(herr, "Unexpected %s in this scope");
        break;
      
      case TT_ENDOFCODE:
        return 0;
    }
    token = read_next_token(scope);
  }
}
