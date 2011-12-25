/**
 * @file handle_declarators.cpp
 * @brief Source implementing the parser function to handle standard declarations.
 * 
 * This file's function will be referenced by every other function in the
 * parser. The efficiency of its implementation is of crucial importance.
 * If this file runs slow, so do the others in the parser.
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

#include "../bodies.h"
#include "../../System/context.h"
#include "../../General/parse_basics.h"
#include "../../General/debug_macros.h"
#include "../parse_context.h"
#include <cstdio>
using namespace jdip;
using namespace jdi;

/**
  @section Implementation
  This implementation is relatively unremarkable, save for the bouts
  of dancing around llreader's structure. Normally, I would write such
  a function to play on null-terminated C strings, however, due to the
  variety of memory constructs an llreader may actually represent, that
  isn't actually an option here.
**/
int jdip::context_parser::handle_declarators(llreader &cfile, definition_scope *scope, token_t& token, parse_context &pc)
{
  // Outsource to read_type, which will take care of the hard work for us.
  // When this function finishes, per its specification, our token will be set to the next relevant, non-referencer symbol.
  // This means an identifier if the syntax is correct.
  full_type tp = read_type(cfile, token, scope, pc);
  
  // Make sure we actually read a valid type.
  if (!tp.def) {
    pc.error = "Declaration does not give a valid type";
    return 1;
  }
  
  // Make sure we do indeed find ourselves at an identifier to declare.
  if (token.type != TT_IDENTIFIER) {
    pc.error = "Declaration doesn't declare anything";
    return 2;
  }
  
  // Take record of the name.
  string name((const char*)token.extra.content.str,token.extra.content.len);
  
  // Add it to our definitions map, without overwriting the existing member.
  definition_scope::inspair ins = ((definition_scope*)scope)->members.insert(definition_scope::entry(name,NULL));
  if (ins.second) // If we successfully inserted,
    ins.first->second = new definition_typed(name,scope,tp.def,tp.refs);
  #ifndef NO_ERROR_REPORTING
  else // Well, uh-the-fuck-oh. We didn't insert anything. This is non-fatal, and will not leak, so no harm done.
  {
    if (not(ins.first->second->flags & DEF_TYPED)) {
      pc.error = "Redeclaration of `" + name + "' as a different kind of symbol";
      return 3;
    }
    if (not(ins.first->second->flags & DEF_TYPED) & DEF_EXTERN) { //TODO: Implement
      pc.error = "Redeclaration of non-extern `" + name + "' as non-extern";
      return 4;
    }
  }
  #endif
  
  token = read_next_token(cfile, scope, pc);
  switch (token.type) {
    case TT_OPERATOR:
        if (*token.extra.content.str != '=' or token.extra.content.len != 1) { // If this operator isn't =, this is a fatal error. No idea where we are.
          pc.error = "Unexpected operator " + string((const char*)token.extra.content.str,token.extra.content.len) + " at this point";
          return 5;
        }
        // If this thing's const, we need to make note of the value... FML
        cout << "UNIMPLEMENTED: Initializers" << endl;
      break;
    case TT_COMMA:
        // Clear the refstack from our previous type; ISO C says that char *foo, bar; makes foo a char* and bar a char.
        tp.refs.clear();
        
      break;
  }
  
  return 0;
}

