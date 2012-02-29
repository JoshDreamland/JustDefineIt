/**
 * @file lex_cpp.h
 * @brief Header extending the \c lexer base class for parsing C++.
 * 
 * This file defines two subclasses of \c jdi::lexer. The first is meant to lex
 * C++ definitions, and so returns a wide range of token types. It will also do
 * any needed preprocessing, handing the second lexer to \c jdi::AST to handle
 * #if expression evaluation. The second lexer is much simpler, and treats 
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

#include <API/lexer_interface.h>
#include <General/quickstack.h>
#include <General/llreader.h>
#include <Parser/bodies.h>
#include <API/context.h>

namespace jdip {
  using namespace jdi;
  /**
    @brief An extension of \c llreader which also stores information about the
           current line number and the position of the last line break.
  **/
  struct openfile: llreader {
    string filename; ///< The name of the open file.
    size_t line, ///< The index of the current line.
           lpos; ///< The position of the most recent line break.
  };
  
  /**
    @brief An implementation of \c jdi::lexer for lexing C++. Handles preprocessing
           seamlessly, returning only relevant tokens.
  **/
  struct lexer_cpp: lexer, openfile {
    token_t get_token(error_handler *herr = def_error_handler);
    quick::stack<openfile> files; ///< The files we have open, in the order we included them.
    macro_map &macros; ///< Reference to the \c jdi::macro_map which will be used to store and retrieve macros.
    
    typedef map<string,TOKEN_TYPE> keyword_map; ///< Map of string to token type; a map-of-keywords type.
    /// List of C++ keywords, mapped to the type of their token.
    /// This list is assumed to contain tokens whose contents are unambiguous; one string maps to one token, and vice-versa.
    keyword_map keywords;
    
    /** Sole constructor; consumes an llreader.
        @param input    The file from which to read definitions. This file will be manipulated by the system.
        @param pmacros  A \c jdi::macro_map which will receive and be probed for macros.
    **/
    lexer_cpp(llreader& input, macro_map &pmacros);
  };
  
  /**
    @brief An implementation of \c jdi::lexer for handling macro expressions.
           Unrolls macros automatically. Treats non-macro identifiers as zero.
  **/
  struct lexer_macro: lexer {
    token_t get_token(error_handler *herr = def_error_handler);
    quick::stack<openfile*> files; ///< The files we have open                                                              
  };
}
