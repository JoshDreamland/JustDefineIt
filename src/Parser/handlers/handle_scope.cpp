/**
 * @file  bodies.h
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

#include "../bodies.h"

int jdip::context_parser::handle_scope(llreader &cfile, definition_scope *scope, token_t& token)
{
  token = read_next_token(cfile, scope);
  for (;;)
  {
    switch (token.type) {
      case TT_DECLARATOR:
          if (handle_declarators(cfile, scope, token))
            return 1;
          if (token.type != TT_SEMICOLON)
            return (token.report_error(pc, "Expected semicolon at this point"), 1);
        break;
      
      case TT_COMMA:
          token.report_error(pc, "Unexpected comma at this point.");
        return 1;
      
      case TT_SEMICOLON:
          /* Printing a warning here is advisable but unnecessary. */
        break;
      
      case TT_CLASS:
      case TT_STRUCT:
      case TT_ENUM:
      case TT_UNION:
      case TT_NAMESPACE:
      case TT_IDENTIFIER:
      case TT_TEMPLATE:
      case TT_TYPENAME:
      case TT_OPERATOR:
      case TT_TYPEDEF:
      case TT_USING:
      case TT_PUBLIC:
      case TT_PRIVATE:
      case TT_PROTECTED:
      case TT_COLON:
      case TT_SCOPE:
      case TT_LEFTPARENTH:
      case TT_RIGHTPARENTH:
      case TT_LEFTBRACKET:
      case TT_RIGHTBRACKET:
      case TT_LEFTBRACE:
      case TT_RIGHTBRACE:
      case TT_DESTRUCTOR:
      case TT_EQUALS:
      case TT_STRINGLITERAL:
      case TT_DECLITERAL:
      case TT_HEXLITERAL:
      case TT_OCTLITERAL:
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
    token = read_next_token(cfile, scope);
  }
}
