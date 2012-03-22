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

#ifndef _LEX_CPP__H
#define _LEX_CPP__H

namespace jdip {
  struct lexer_cpp;
  struct lexer_macro;
}

#include <API/lexer_interface.h>
#include <General/quickstack.h>
#include <General/llreader.h>
#include <API/context.h>

namespace jdip {
  using namespace jdi;
  /**
    @brief An extension of \c llreader which also stores information about the
           current line number and the position of the last line break.
  **/
  struct openfile {
    string filename; ///< The name of the open file.
    size_t line; ///< The index of the current line.
    size_t lpos; ///< The position of the most recent line break.
    llreader file; ///< The llreader of this file.
    openfile(); ///< Default constructor.
    openfile(string fname); ///< Construct a new openfile at position 0 with the given filename.
    void swap(openfile&); ///< Copy constructor.
  };
  
  /**
    @brief An implementation of \c jdi::lexer for lexing C++. Handles preprocessing
           seamlessly, returning only relevant tokens.
  **/
  struct lexer_cpp: lexer, llreader {
    token_t get_token(error_handler *herr = def_error_handler);
    quick::stack<openfile> files; ///< The files we have open, in the order we included them.
    macro_map &macros; ///< Reference to the \c jdi::macro_map which will be used to store and retrieve macros.
    
    typedef map<string,TOKEN_TYPE> keyword_map; ///< Map of string to token type; a map-of-keywords type.
    /// List of C++ keywords, mapped to the type of their token.
    /// This list is assumed to contain tokens whose contents are unambiguous; one string maps to one token, and vice-versa.
    keyword_map keywords;
    
    /** Sole constructor; consumes an llreader and attaches a new \c lex_macro.
        @param input    The file from which to read definitions. This file will be manipulated by the system.
        @param pmacros  A \c jdi::macro_map which will receive and be probed for macros.
    **/
    lexer_cpp(llreader& input, macro_map &pmacros);
    /** Destructor; free the attached macro lexer. **/
    ~lexer_cpp();
    
    /**
      Utility function designed to handle the preprocessor directive
      pointed to by \c pos upon invoking the function. Note that it should
      be the character directly after the pound pointed to upon invoking
      the function, not the pound itself.
      @param herr  The error handler to use if the preprocessor doesn't
                   exist or is malformed.
    **/
    void handle_preprocessor(error_handler *herr);
    /**
      Second-order utility function to skip lines until a preprocessor
      directive is encountered, then invoke the handler on the directive it found.
    **/
    void skip_to_macro(error_handler(*herr));
    
  private:
    /// Storage mechanism for conditionals, such as #if, #ifdef, and #1ifndef
    struct condition {
      bool is_true; ///< True if code in this layer is to be parsed; ie, the condition given is true.
      bool can_be_true; ///< True if an `else` statement or the like can set is_true to true.
      condition(bool,bool); ///< Convenience constructor.
      condition(); ///< Default constructor.
    };
    quick::stack<condition> conditionals; ///< Our conditional levels (one for each nested #if*)
    lexer_macro *mlex; ///< The macro lexer that will be passed to the AST builder for #if directives.
  };
  
  /**
    An implementation of \c jdi::lexer for handling macro expressions.
    Unrolls macros automatically. Treats non-macro identifiers as zero.
    Replaces `defined x` with 0 or 1, depending on whether x is defined.
  **/
  struct lexer_macro: lexer {
    const char* cfile;
    size_t &pos, length;
    lexer_cpp *lcpp;
    token_t get_token(error_handler *herr = def_error_handler);
    lexer_macro(lexer_cpp*);
    void update();
  };
}

#endif