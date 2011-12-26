/**
 * @file  base.cpp
 * @brief The source implementing the top-level function of the parser.
 * 
 * Very little work is done by this file; it's mostly the front-end, but such
 * is usually the case in a recursive-descent scheme. The job of this file is
 * simply to delegate to other files.
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
**/

#include <fstream>
#include "../System/context.h"
#include "../System/token.h"
#include "../General/debug_macros.h"
#include "parse_context.h"
#include "bodies.h"
using namespace std;
using namespace jdip;

/**
  @brief Parse an input stream for definitions.
  
  This function's only purpose is to make a call to fetch the next token, figure
  out what it represents, and dispatch a new function to deal with it. The vast
  majority of the work is done in the function that grabs the next token.
  
  @param cfile  The stream to be read in.
**/
int jdi::context::parse_C_stream(llreader &cfile)
{
  stack_tracer("void jdi::context::parse_C_stream(llreader &cfile)");
  if (pc) {
    if (pc->active) {
      pc->error = "STILL PARSING";
      return -1;
    }
    delete pc;
  }
  pc = new parse_context();
  context_parser* const p = (context_parser*)this;
  
  struct deactivate { parse_context* &pc;
    deactivate(parse_context*& p): pc(p) {}
    ~deactivate() { pc->active = false; }
  } deactivate_me(pc);
  
  for (;;) {
    token_t tk = p->read_next_token(cfile, global);
    switch (tk.type) {
      case TT_DECLARATOR:
        if (p->handle_declarators(cfile, global, tk))
          return 1;
        break;
      
      case TT_COMMA:
          tk.report_error(pc,"Unexpected comma at this point.");
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
      case TT_LEFTTRIANGLE:
      case TT_RIGHTTRIANGLE:
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
          cout << TOKEN_TYPE_NAME[tk.type] << endl;
        #endif
        break;
      case TT_ENDOFCODE:
        return 0;
    }
  }
}
