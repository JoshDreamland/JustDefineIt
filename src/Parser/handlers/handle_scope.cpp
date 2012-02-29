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

int jdip::context_parser::handle_scope(lexer *lex, definition_scope *scope, token_t& token)
{
  token = read_next_token(lex, scope);
  for (;;)
  {
    switch (token.type) {
      case TT_DECLARATOR: case TT_DECFLAG:
          if (handle_declarators(lex, scope, token))
            return 1;
          if (token.type != TT_SEMICOLON)
            return (token.report_error(this, "Expected semicolon at this point"), 1);
        break;
      
      case TT_COMMA:
          token.report_error(this, "Unexpected comma at this point.");
        return 1;
      
      case TT_SEMICOLON:
          /* Printing a warning here is advisable but unnecessary. */
        break;
      
      case TT_NAMESPACE: {
        token = read_next_token(lex, scope);
        if (token.type != TT_IDENTIFIER) {
          token.report_error(this, "Expected namespace name here.");
          return 1;
        }
        
        // Copy the name and ensure it's a member of this scope.
        string nsname((const char*)token.extra.content.str, token.extra.content.len);
        pair<definition_scope::defiter, bool> dins = scope->members.insert(pair<string,definition*>(nsname,NULL));
        
        definition_scope *nscope;
        if (dins.second) // If a new definition key was created, then allocate a new namespace representation for it.
          dins.first->second = nscope = new definition_scope(nsname,scope,DEF_NAMESPACE);
        else {
          nscope = (definition_scope*)dins.first->second;
          if (not(dins.first->second->flags & DEF_NAMESPACE)) {
            token.report_error(this,"Attempting to redeclare `" + nsname + "' as a namespace");
            return 1;
          }
        }
        
        token = read_next_token(lex,scope);
        if (token.type != TT_LEFTBRACE) {
          token.report_error(this,"Expected opening brace for namespace definition.");
          return 1;
        }
        if (handle_scope(lex, nscope, token)) return 1;
        if (token.type != TT_RIGHTBRACE) {
          token.report_error(this,"Expected closing brace to namespace `" + nsname + "'");
          return 1;
        }
        break;
      }
      
      case TT_LEFTPARENTH:  token.report_error(this, "Stray opening parenthesis."); return 1;
      case TT_RIGHTPARENTH: token.report_error(this, "Stray closing parenthesis."); return 1;
      case TT_LEFTBRACKET:  token.report_error(this, "Stray opening bracket."); return 1;
      case TT_RIGHTBRACKET: token.report_error(this, "Stray closing bracket."); return 1;
      case TT_LEFTBRACE:    token.report_error(this, "Expected scope declaration before opening brace."); return 1;
      case TT_RIGHTBRACE:   return 0;
      
      case TT_CLASS: case TT_STRUCT: case TT_ENUM: case TT_UNION:
      case TT_TYPENAME:
      
      case TT_IDENTIFIER:
      
      case TT_OPERATOR: case TT_LESSTHAN: case TT_GREATERTHAN:
      case TT_COLON:
      
      case TT_SCOPE:
      case TT_TILDE:
      case TT_EQUALS:
      case TT_STRINGLITERAL:
      
      case TT_DECLITERAL:
      case TT_HEXLITERAL:
      case TT_OCTLITERAL:
      
      case TT_TYPEDEF: case TT_USING: case TT_PUBLIC: case TT_PRIVATE: case TT_PROTECTED:
      case TT_TEMPLATE:
      
      case TT_INVALID:
      default:
        cout << "ERROR: INVALID TOKEN TYPE RETURNED.\n";
        #ifdef DEBUG_MODE
          cout << TOKEN_TYPE_NAME[token.type] << endl;
        #endif
        break;
      case TT_ENDOFCODE:
        return 0;
    }
    token = read_next_token(lex, scope);
  }
}
