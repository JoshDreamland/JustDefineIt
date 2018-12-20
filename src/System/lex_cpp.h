/**
 * @file lex_cpp.h
 * @brief Header extending the \c lexer base class for parsing C++.
 * 
 * This file defines two subclasses of \c jdi::lexer. The first is meant to lex
 * C++ definitions, and so returns a wide range of token types. It will also do
 * any needed preprocessing, handing the second lexer to \c jdi::AST to handle
 * `#if` expression evaluation. The second lexer is much simpler, and treats all
 * identifiers the same.
 * 
 * @section License
 * 
 * Copyright (C) 2011-2014 Josh Ventura
 * This file is part of JustDefineIt.
 * I only code for 
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

namespace jdi {
  struct lexer;
  struct lexer_macro;
}

#include <set>
#include <string>
#include <vector>
#include <General/quickstack.h>
#include <General/llreader.h>
#include <System/token.h>
#include <System/macros.h>

namespace jdi {
  using std::string;
  
  struct file_meta {
    string name; ///< The name of the open file or macro.
    string searchdir; ///< The search directory from which this file was included, or the empty string.
    size_t from_line; ///< The index of the line in the file that used or included this one.
    size_t from_lpos; ///< The position in from_line.
  };
  
  struct openfile: file_meta {
    llreader file;
  };
  
  struct EnteredMacro: file_meta {
    const std::vector<token_t> &tokens;
    std::vector<token_t> assembled_token_data;
    EnteredMacro(string macro, size_t line, size_t lpos,
                 const std::vector<token_t> *tokens_):
                     file_meta{macro, "", line, lpos},
                     tokens(*tokens_) {}
    EnteredMacro(string macro, size_t line, size_t lpos,
                 std::vector<token_t> &&tokens_):
                     file_meta{macro, "", line, lpos},
                     tokens(assembled_token_data),
                     assembled_token_data(std::move(tokens_)) {}
  };
  
  /// The basic C++ lexer. Extracts a single preprocessor token from the given
  /// reader. Uses @p file to include source metadata in the token.
  token_t read_token(llreader &data, const file_meta &file,
                     error_handler *herr = def_error_handler);

  /**
   * An implementation of \c jdi::lexer for lexing C++.
   * Handles preprocessing seamlessly, returning only relevant tokens.
   **/
  struct lexer {
    llreader cfile;  ///< The current file being read.
    std::vector<openfile> files; ///< The macros we aI need to fix the re nested in and files we have open, in the order we entered them.
    std::vector<EnteredMacro> open_macros; ///< Macros we are currently nested in.
    error_handler *herr;  ///< Error handler for problems during lex.
    
    /// Tokens that have been expanded from a macro or fetched as lookahead.
    std::vector<token_t> *buffered_tokens = nullptr;
    /// The position in the current token buffer.
    size_t buffer_pos;
    
    /// Buffer to which tokens will be recorded for later re-parse, as needed.
    std::vector<token_t> *lookahead_buffer = nullptr;
    /// RAII type for initiating a lookahead.
    struct look_ahead {
      std::vector<token_t> buffer;
      std::vector<token_t> *prev_buffer;
      lexer *lex;
      
      token_t &push(token_t token) {
        buffer.push_back(token);
        return buffer.back();
      }
      
      look_ahead(lexer *lex_): prev_buffer(lex->lookahead_buffer), lex(lex_) {
        lex->lookahead_buffer = &buffer;
      }
      ~look_ahead() {
        if (lex->lookahead_buffer != &buffer) {
          lex->herr->error("LOGIC ERROR: lookahead buffer is not owned");
          abort();
        }
        lex->lookahead_buffer = prev_buffer;
      }
    };
    
    macro_map &macros; ///< Reference to the \c jdi::macro_map which will be used to store and retrieve macros.
    
    /// Read a raw token; this implies that TT_IDENTIFIER is the only token returned when any id is encountered: no keywords, no declarators, no definitions.
    token_t read_raw();
    /// Read a C++ token, with no scope information.
    token_t get_token();
    
    
    /// Enter a scalar macro, if it has any content.
    /// @param ms   The macro scalar to enter.
    void enter_macro(macro_type *ms);
    /// Parse for parameters to a given macro function, if there are any, then evaluate
    /// the macro function and set the open file to reflect the change.
    /// This call should be made while the position is just after the macro name.
    /// @param mf   The macro function to parse
    /// @param herr An error handler in case of parameter mismatch or non-terminated literals
    /// @return Returns whether parameters were encountered and parsed.
    bool parse_macro_function(const macro_type* mf);
    /// Parse for parameters to a given macro function, if there are any.
    /// This call should be made while the position is just after the macro name.
    /// @param mf    The macro function to parse.
    /// @param dest  The vector to receive the individual parameters [out].
    /// @param herr  An error handler in case of parameter mismatch or non-terminated literals.
    /// @return Returns whether parameters were encountered and parsed.
    bool parse_macro_params(const macro_type* mf, vector<string>& dest);
    
    /// Utility function to skip a single-line comment; invoke with pos indicating one of the slashes.
    inline void skip_comment();
    /// Utility function to skip a multi-line comment; invoke with pos indicating the starting slash.
    inline void skip_multiline_comment();
    /// Utility function to skip a string; invoke with pos indicating the quotation mark. Terminates indicating match.
    inline void skip_string();
    /// Skip anything that cannot be interpreted as code in any way.
    inline void skip_whitespace();
    
    /// Pop the currently open file or active macro.
    /// @return Returns whether the end of all input has been reached.
    bool pop_file();
    
    /// Map of string to token type; a map-of-keywords type.
    typedef map<string,TOKEN_TYPE> keyword_map;
    /// List of C++ keywords, mapped to the type of their token.
    /// This list is assumed to contain tokens whose contents are unambiguous; one string maps to one token, and vice-versa.
    static keyword_map keywords;
    /// This is a map of macros to add bare-minimal support for a number of compiler-specific builtins.
    static macro_map kludge_map;
    
    /// Static cleanup function; safe to call without a matching init.
    static void cleanup();
    
    /** Sole constructor; consumes an llreader and attaches a new \c lex_macro.
        @param input    The file from which to read definitions. This file will be manipulated by the system.
        @param pmacros  A \c jdi::macro_map which will receive and be probed for macros.
        @param fname    The name of the file that was first opened.
    **/
    lexer(llreader& input, macro_map &pmacros, const char *fname = "stdcall/file.cpp");
    /** Destructor; free the attached macro lexer. **/
    ~lexer();
    
    /**
      Utility function designed to handle the preprocessor directive
      pointed to by \c pos upon invoking the function. Note that it should
      be the character directly after the pound pointed to upon invoking
      the function, not the pound itself.
      @param herr  The error handler to use if the preprocessor doesn't
                   exist or is malformed.
    **/
    void handle_preprocessor(error_handler *herr);
    
    /// Function used by the preprocessor to read in macro parameters in compliance with ISO.
    string read_preprocessor_args(error_handler *herr);
    /** Second-order utility function to skip lines until a preprocessor
        directive is encountered, then invoke the handler on the directive it found. **/
    void skip_to_macro(error_handler *herr);
    
    set<string> visited_files; ///< For record and reporting purposes only.
    
  protected:
    /// Storage mechanism for conditionals, such as <code>\#if</code>, <code>\#ifdef</code>, and <code>\#ifndef</code>.
    struct condition {
      bool is_true; ///< True if code in this layer is to be parsed; ie, the condition given is true.
      bool can_be_true; ///< True if an `else` statement or the like can set is_true to true.
      condition(bool,bool); ///< Convenience constructor.
      condition(); ///< Default constructor.
    };
    
    quick::stack<condition> conditionals; ///< Our conditional levels (one for each nested `\#if*`)
    lexer_macro *mlex; ///< The macro lexer that will be passed to the AST builder for #if directives.
    context_parser *mctex; ///< A context used for constructing ASTs from preprocessor expressions.
  };
}

#endif
