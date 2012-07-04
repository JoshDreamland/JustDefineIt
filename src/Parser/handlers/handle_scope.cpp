/**
 * @file  handle_scope.cpp
 * @brief Source implementing a massive delegator which populates a scope.
 * 
 * This file does a huge amount of work.
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
          token = read_next_token(global);
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
      
      case TT_TEMPLATE:
        if (handle_template(scope, token, inherited_flags))
          return 1;
        break;
      
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
