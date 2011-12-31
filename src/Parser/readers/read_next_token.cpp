/**
 * @file read_next_token.cpp
 * @brief Source implementing the parser function to fetch the next token.
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

#define cfile c_file // Makes the code look prettier without making doxygen bitch
/**
  @section Implementation
  This implementation is relatively unremarkable, save for the bouts
  of dancing around llreader's structure. Normally, I would write such
  a function to play on null-terminated C strings. However, due to the
  variety of memory constructs an llreader may actually represent, that
  isn't an option here.
**/
token_t jdip::context_parser::read_next_token(llreader &cfile, definition_scope *scope)
{
  #undef cfile
  stack_tracer("token_t jdip::read_next_token(llreader &c_file, parse_context &pc)");
  size_t &pos = c_file.pos; // Save on access times
  const char* &cfile = c_file.data; // Save one dereference
  size_t &len = c_file.length;
  
  if (pos >= len) goto POP_FILE;
  for (;;) // Loop until we find something or hit world's end
  {
    loop_tracer();
    
    // Skip all whitespace
    while (is_useless(cfile[pos])) if (++pos >= len) goto POP_FILE;
    
    // Check if we're at a comment
    if (cfile[pos] == '/') {
      if (++pos < len) {
        if (cfile[pos] == '/') {
          // Find the end of the line
          while (cfile[pos] != '\n') if (++pos >= len) goto POP_FILE;
          continue;
        }
        if (cfile[pos] == '*') {
          ++pos; // Skip one more char so we don't break on /*/
          do if (++pos >= len) goto POP_FILE; while (cfile[pos] != '/' or cfile[pos-1] != '*');
          continue;
        }
        if (cfile[pos] == '=')
          return token_t(token_basics(TT_OPERATOR,"some file",0,pos), cfile+pos-1, 2);
      }
      return token_t(token_basics(TT_OPERATOR,"some file",0,pos), cfile+pos-1,1);
    }
    
    /*-- Guess we're not. Figure out where we are. --------------------*\
    \*-----------------------------------------------------------------*/
    
    if (is_letter(cfile[pos])) // Check if we're at an identifier.
    {
      const char* sp = cfile + pos; // Record where we are
      while (++pos < len and is_letterd(cfile[pos]));
      
      string fn(sp, cfile + pos); // We'll need a copy of this thing for lookup purposes
      
      macro_iter mi = macros.find(fn);
      if (mi != macros.end()) {
        printf("ERROR: Unimplemented: macros\n");
      }
      
      return look_up_token(scope,fn,token_t(token_basics(TT_IDENTIFIER,"some file",0,0), sp, fn.length()));
    }
    
    if (is_digit(cfile[pos])) {
      if (cfile[pos++] == '0') {
        if (cfile[pos] == 'x') {
          const size_t sp = pos;
          while (++pos < len and is_hexdigit(cfile[pos]));
          while (pos < len and is_letter(cfile[pos])) pos++; // Skip the ull and shit
          return token_t(token_basics(TT_HEXLITERAL,"some file",0,pos), cfile+sp, pos-sp);  
        }
        const size_t sp = pos;
        while (++pos < len and is_hexdigit(cfile[pos]));
        while (pos < len and is_letter(cfile[pos])) pos++; // Skip the ull and shit
        return token_t(token_basics(TT_OCTLITERAL,"some file",0,pos), cfile+sp, pos-sp);
      }
      const size_t sp = pos;
      while (pos < len and is_digit(cfile[pos])) pos++;
      while (pos < len and is_letter(cfile[pos])) pos++; // Skip the ull and shit
      return token_t(token_basics(TT_DECLITERAL,"some file",0,pos), cfile+sp, pos-sp);
    }
    
    const size_t spos = pos;
    switch (cfile[pos++])
    {
      case ';':
        return token_t(token_basics(TT_SEMICOLON,"some file",0,spos));
      case ',':
        return token_t(token_basics(TT_COMMA,"some file",0,spos));
      case '+': case '-':
        pos += cfile[pos] == cfile[spos] or cfile[pos] == '=';
        return token_t(token_basics(TT_OPERATOR,"some file",0,spos), cfile+spos, pos-spos);
      case '*': case '/': case '^': case '=':
        pos += cfile[pos] == cfile[spos];
        return token_t(token_basics(TT_OPERATOR,"some file",0,spos), cfile+spos, pos-spos);
      case '&': case '|':  case '!': case '~': 
        pos += cfile[pos] == cfile[spos] || cfile[pos] == '=';
        return token_t(token_basics(TT_OPERATOR,"some file",0,spos), cfile+spos, pos-spos);
      case '>': case '<':
        pos += cfile[pos] == cfile[spos]; pos += cfile[pos] == '=';
        return token_t(token_basics(TT_OPERATOR,"some file",0,spos), cfile+spos, pos-spos);
      default:
        return token_t(token_basics(TT_INVALID,"some file",0,pos++));
    }
  }
  
  return token_t();
  
  POP_FILE: // This block was created instead of a helper function to piss Rusky off.
  if (pc->files.empty())
    return token_t(token_basics(TT_ENDOFCODE,"some file",0,pos));
  return read_next_token(c_file, scope);
}

token_t jdip::context_parser::look_up_token(definition_scope* scope, string name, token_t defalt)
{
  definition *def = scope->look_up(name);
  if (!def) return defalt;
  
  if (def->flags & DEF_TYPENAME) {
    defalt.extra.def = def;
    defalt.type = TT_DECLARATOR;
    return defalt;
  }
  
  return defalt;
}
