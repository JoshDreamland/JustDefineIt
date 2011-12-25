/**
 * @file token.h
 * @brief Header defining token types.
 * 
 * These are system constants; use only if you have read the \c jdip disclaimer.
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

#ifndef _TOKEN__H
#define _TOKEN__H

#include <string>
#include "../Parser/parse_context.h"

namespace jdip {
  enum TOKEN_TYPE {
    TT_DECLARATOR, ///< Phrases like const, long, int, struct {}; anything that can be used as a type. 
    TT_CLASS,      ///< The `class' keyword.
    TT_STRUCT,     ///< The `struct' keyword.
    TT_ENUM,       ///< The `enum' keyword.
    TT_UNION,      ///< The `union' keyword.
    TT_NAMESPACE,  ///< The `namespace' keyword.
    
    TT_IDENTIFIER, ///< A standard identifier.
    
    TT_TEMPLATE,   ///< The `template' keyword, which should be followed by <...>
    TT_TYPENAME,   ///< The `typename' keyword.
    
    TT_OPERATOR,   ///< The `operator' keyword, not a random operator.
    TT_TYPEDEF,    ///< The `typedef' keyword.
    TT_USING,      ///< The `using' keyword.
    
    TT_PUBLIC,     ///< The `public' keyword.
    TT_PRIVATE,    ///< The `private' keyword.
    TT_PROTECTED,  ///< The `protected' keyword.
    
    TT_COLON,      ///< A simple colon, which should always mark a label.
    TT_SCOPE,      ///< The scope accessor `::' symbol.
    
    TT_LEFTPARENTH,   ///< A left parenthesis, `('.
    TT_RIGHTPARENTH,  ///< A right parenthesis, `)'.
    TT_LEFTBRACKET,   ///< A left bracket, `['.
    TT_RIGHTBRACKET,  ///< A right bracket, `]'.
    TT_LEFTBRACE,     ///< A left brace, `{'.
    TT_RIGHTBRACE,    ///< A right brace, `}'.
    TT_LEFTTRIANGLE,  ///< The less-than symbol `<'. Implies we're at a template.
    TT_RIGHTTRIANGLE, ///< The greater-than symbol `>'. Implies we're ending a template.
    
    TT_DESTRUCTOR,    ///< The tilde `~' symbol.
    TT_COMMA,         ///< A comma, `,'. Separates items in lists.
    TT_SEMICOLON,     ///< A semicolon, `;'. Separates statements and declarations.
    TT_EQUALS,        ///< An equals sign, `='. Used for assigning default values and const values.
    
    TT_STRINGLITERAL, ///< A string literal, such as "hello, world!"
    TT_DECLITERAL,    ///< A decimal literal, such as 12345
    TT_HEXLITERAL,    ///< A hexadecimal literal, such as 0xDEC0DED
    TT_OCTLITERAL,    ///< An octal literal, such as 07654321.
    
    TT_ENDOFCODE, ///< This token signifies that the code has reached its end.
    TT_INVALID ///< Invalid token; read failed.
  };
  
  #ifdef DEBUG_MODE
    /// A debug listing of token descriptions by value ID
    extern const char* TOKEN_TYPE_NAME[];
  #endif
  
  /**
    A structure for representing complete info about a token.
    First and foremost, the structure denotes the type of the token. If that
    type is insufficient to discern the text of the token, the text is included
    as well; otherwise, there is little point to maintaining a copy in memory.
    For reasons of error reporting, the structure also contains information on
    where the token originated: the filename, and the line number/position.
  **/
  struct token_t {
    TOKEN_TYPE type; ///< The type of this token
    /*const char* content; ///< Pointer to the beginning of the text of this token, where it can otherwise not be discerned from the type.
    int length; ///< The length of the data pointed to by \c content.*/
    
    /// Construct a new, invalid token.
    token_t();
    #ifndef NO_ERROR_REPORTING
      /// Log the name of the file from which this token was read.
      volatile const char* file;
      /// Log the line on which this token was named in the file.
      int linenum;
      #ifndef NO_ERROR_POSITION
        /// We are logging positions for precise error reporting.
        int pos;
        
        /// Construct a new token with the file, line number, and position in which it was declared.
        #define full_token_constructor_parameters int, const char*, int, int
        /// Wrapper to the token_t constructor which can ignore removed attributes.
        #define token_basics(type,filename,line,pos) type,filename,line,pos
      #else
        /// Wrapper to the token_t constructor which can ignore removed attributes.
        #define token_basics(type,filename,line,pos) type,filename,line
        /// Construct a new token with the file and line number in which it was declared.
        #define full_token_constructor_parameters int, const char*, int
      #endif
      /// Construct a new token without origin information.
      token_t(TOKEN_TYPE t, const char* fn, int l);
    #else
      /// Wrapper to the token_t constructor which can ignore removed attributes.
      #define token_basics(type,filename,line,pos) type
      #define full_token_constructor_parameters TOKEN_TYPE, const char*, int
    #endif
    
    /** A simple union detailing more information about the token.
        This union is meant to contain either data about what text constituted
        the token or what definition was found to justify the token, or neither
        depending on how self-explanatory the type of the token is. **/
    union extra_ {
      struct {
        /// A pointer to a substring of a larger buffer of code. NEITHER is null-terminated!
        /// This pointer is to be considered volatile as the buffer belongs to the system and
        /// can be modified or freed as soon as the file is closed. As such, any use of it must
        /// be made before the file is closed.
        volatile const char* str;
        
        /// The length of the string pointed to by \c content.
        int len;
      } content;
      
      /// For types, namespace-names, etc., the definition by which the type of this token was determined.
      definition* def;
      
      extra_();
      extra_(const char*, int);
      extra_(definition*);
    } extra;
    
    /// Construct a token with the requested amount of information.
    token_t(token_basics(TOKEN_TYPE t, const char* fn, int l, int p));
    /// Construct a token with extra information regarding its content.
    token_t(token_basics(TOKEN_TYPE t, const char* fn, int l, int p), const char*, int);
    /// Construct a token with extra information regarding its definition.
    token_t(token_basics(TOKEN_TYPE t, const char* fn, int l, int p), definition*);
    
    /**
      Copy error information to a parse context.
      If no information is available, then zeros are copied in its place.
      @param pc    The context into which error info is copied.
      @param error The text of the error.
    **/
    void report_error(parse_context& pc, std::string error);
  };
  
  
}

#endif
