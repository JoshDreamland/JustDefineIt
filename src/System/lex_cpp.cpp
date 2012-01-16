/**
 * @file lex_cpp.cpp
 * @brief Source implementing the C++ \c lexer class extensions.
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

#include "lex_cpp.h"
#include <General/debug_macros.h>
#include <General/parse_basics.h>
#include <Parser/parse_context.h>
#include <System/context.h>
#include <Parser/bodies.h>
#include <cstdio>

using namespace jdi;
using namespace jdip;

token_t lexer_cpp::get_token(context* ct, definition_scope* scope, error_handler *herr)
{
  stack_tracer("token_t jdip::read_next_token(llreader &c_file, parse_context &pc)");
  #define cfile data
  
  if (!scope) scope = ct->global;
  if (pos >= length) goto POP_FILE;
  for (;;) // Loop until we find something or hit world's end
  {
    loop_tracer();
    
    // Skip all whitespace
    while (is_useless(cfile[pos])) if (++pos >= length) goto POP_FILE;
    
    // Check if we're at a comment
    if (cfile[pos] == '/') {
      if (++pos < length) {
        if (cfile[pos] == '/') {
          // Find the end of the line
          while (cfile[pos] != '\n') if (++pos >= length) goto POP_FILE;
          continue;
        }
        if (cfile[pos] == '*') {
          ++pos; // Skip one more char so we don't break on /*/
          do if (++pos >= length) goto POP_FILE; while (cfile[pos] != '/' or cfile[pos-1] != '*');
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
      while (++pos < length and is_letterd(cfile[pos]));
      
      string fn(sp, cfile + pos); // We'll need a copy of this thing for lookup purposes
      
      macro_iter mi = ct->macros.find(fn);
      if (mi != ct->macros.end()) {
        printf("ERROR: Unimplemented: macros\n");
      }
      
      return ((context_parser*)ct)->look_up_token(scope,fn,token_t(token_basics(TT_IDENTIFIER,"some file",0,0), sp, fn.length()));
    }
    
    if (is_digit(cfile[pos])) {
      if (cfile[pos++] == '0') {
        if (cfile[pos] == 'x') {
          const size_t sp = pos;
          while (++pos < length and is_hexdigit(cfile[pos]));
          while (pos < length and is_letter(cfile[pos])) pos++; // Skip the ull and shit
          return token_t(token_basics(TT_HEXLITERAL,"some file",0,pos), cfile+sp, pos-sp);  
        }
        const size_t sp = pos;
        while (++pos < length and is_hexdigit(cfile[pos]));
        while (pos < length and is_letter(cfile[pos])) pos++; // Skip the ull and shit
        return token_t(token_basics(TT_OCTLITERAL,"some file",0,pos), cfile+sp, pos-sp);
      }
      const size_t sp = pos;
      while (pos < length and is_digit(cfile[pos])) pos++;
      while (pos < length and is_letter(cfile[pos])) pos++; // Skip the ull and shit
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
      case ':':
        pos += cfile[pos] == cfile[spos];
        return token_t(token_basics(pos - spos == 1 ? TT_COLON : TT_SCOPE,"some file",0,spos), cfile+spos, pos-spos);
      default:
        return token_t(token_basics(TT_INVALID,"some file",0,pos++));
    }
  }
  
  return token_t();
  
  POP_FILE: // This block was created instead of a helper function to piss Rusky off.
  if (files.empty())
    return token_t(token_basics(TT_ENDOFCODE,"some file",0,pos));
  return get_token(ct,scope,herr);
}

lexer_cpp::lexer_cpp(llreader &input) {
  consume(input);
}
