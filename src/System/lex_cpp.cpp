/**
 * @file lex_cpp.cpp
 * @brief Source implementing the C++ \c lexer class extensions.
 * 
 * This file's function will be referenced, directly or otherwise, by every
 * other function in the parser. The efficiency of its implementation is of
 * crucial importance. If this file runs slow, so do the others.
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
#include <System/builtins.h>
#include <API/context.h>
#include <Parser/bodies.h>
#include <cstdio>

using namespace jdi;
using namespace jdip;

token_t lexer_cpp::get_token(error_handler *herr)
{
  stack_tracer("token_t jdip::read_next_token(llreader &c_file, parse_context &pc)");
  #define cfile data //I'm sorry, but I can't spend the whole function calling the file buffer "data."
  
  if (pos >= length) goto POP_FILE;
  for (;;) // Loop until we find something or hit world's end
  {
    loop_tracer();
    
    // Skip all whitespace
    while (is_useless(cfile[pos])) if (++pos >= length) goto POP_FILE;
    
    //============================================================================================
    //====: Check for and handle comments. :======================================================
    //============================================================================================
    
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
    
    //============================================================================================
    //====: Not at a comment. See if we're at an identifier. :====================================
    //============================================================================================
    
    if (is_letter(cfile[pos])) // Check if we're at an identifier or keyword.
    {
      const char* sp = cfile + pos; // Record where we are
      while (++pos < length and is_letterd(cfile[pos]));
      
      string fn(sp, cfile + pos); // We'll need a copy of this thing for lookup purposes
      
      macro_iter mi = macros.find(fn);
      if (mi != macros.end()) {
        printf("ERROR: Unimplemented: macros\n");
      }
      
      keyword_map::iterator kwit = keywords.find(fn);
      if (kwit != keywords.end())
        return token_t(token_basics(kwit->second,"some file",0,0));
      
      tf_iter tfit = builtin_declarators.find(fn);
      if (tfit != builtin_declarators.end()) {
        if ((tfit->second->usage & UF_STANDALONE_FLAG) == UF_PRIMITIVE)
          return token_t(token_basics(TT_DECLARATOR,"some file",0,0), tfit->second->def);
        return token_t(token_basics(TT_DECFLAG,"some file",0,0), (definition*)tfit->second);
      }
      
      return token_t(token_basics(TT_IDENTIFIER,"some file",0,0), sp, fn.length());
    }
    
    //============================================================================================
    //====: Not at an identifier. Maybe at a number? :============================================
    //============================================================================================
    
    if (is_digit(cfile[pos])) {
      if (cfile[pos++] == '0') { // Check if the number is hexadecimal or octal.
        if (cfile[pos] == 'x') { // Check if the number is hexadecimal.
          // Yes, it is hexadecimal.
          const size_t sp = pos;
          while (++pos < length and is_hexdigit(cfile[pos]));
          while (pos < length and is_letter(cfile[pos])) pos++; // Include the flags, like ull
          return token_t(token_basics(TT_HEXLITERAL,"some file",0,pos), cfile+sp, pos-sp);  
        }
        // Turns out, it's octal.
        const size_t sp = pos;
        while (++pos < length and is_hexdigit(cfile[pos]));
        while (pos < length and is_letter(cfile[pos])) pos++; // Include the flags, like ull
        return token_t(token_basics(TT_OCTLITERAL,"some file",0,pos), cfile+sp, pos-sp);
      }
      // Turns out, it's decimal.
      const size_t sp = pos;
      while (pos < length and is_digit(cfile[pos])) pos++;
      while (pos < length and is_letter(cfile[pos])) pos++; // Include the flags, like ull
      return token_t(token_basics(TT_DECLITERAL,"some file",0,pos), cfile+sp, pos-sp);
    }
    
    
    //============================================================================================
    //====: Not at a number. Find out where we are. :=============================================
    //============================================================================================
    
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
        
      case '(': return token_t(token_basics(TT_LEFTPARENTH,"some file",0,spos));
      case '[': return token_t(token_basics(TT_LEFTBRACKET,"some file",0,spos));
      case '{': return token_t(token_basics(TT_LEFTBRACE,  "some file",0,spos));
      case '}': return token_t(token_basics(TT_RIGHTBRACE,  "some file",0,spos));
      case ']': return token_t(token_basics(TT_RIGHTBRACKET,"some file",0,spos));
      case ')': return token_t(token_basics(TT_RIGHTPARENTH,"some file",0,spos));
      
      default:
        return token_t(token_basics(TT_INVALID,"some file",0,pos++));
    }
  }
  
  return token_t();
  
  POP_FILE: // This block was created instead of a helper function to piss Rusky off.
  if (files.empty())
    return token_t(token_basics(TT_ENDOFCODE,"some file",0,pos));
  return get_token(herr);
}
  
lexer_cpp::lexer_cpp(llreader &input, macro_map &pmacros): macros(pmacros) {
  consume(input); // We are also an llreader. Consume the given one using the inherited method.
  keywords["class"] = TT_CLASS;
  keywords["enum"] = TT_ENUM;
  keywords["namespace"] = TT_NAMESPACE;
  keywords["private"] = TT_PRIVATE;
  keywords["protected"] = TT_PROTECTED;
  keywords["public"] = TT_PUBLIC;
  keywords["struct"] = TT_STRUCT;
  keywords["template"] = TT_TEMPLATE;
  keywords["typedef"] = TT_TYPEDEF;
  keywords["typename"] = TT_TYPENAME;
  keywords["union"] = TT_UNION;
  keywords["using"] = TT_USING;
}

