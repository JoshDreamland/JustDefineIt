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

#include <map>
#include <set>
#include <string>
#include <vector>
#include <General/quickstack.h>
#include <General/llreader.h>
#include <System/token.h>
#include <System/macros.h>

namespace jdi {
  using std::string;
  typedef std::vector<token_t> token_vector;

  struct file_meta {
    string name; ///< The name of the open file or macro.
    string searchdir; ///< The search directory from which this file was included, or the empty string.
    size_t from_line; ///< The index of the line in the file that used or included this one.
    size_t from_lpos; ///< The position in from_line.
  };

  struct openfile: file_meta {
    llreader file;
    openfile(llreader &&f): file(f) {}
  };

  struct EnteredMacro {
    /// The name of the macro (used to report errors and prevent recursion).
    string name;
    /// Tokens in this macro (owned by the macro!)
    const token_vector &tokens;
    /// Allows us to own the above vector.
    std::vector<token_t> assembled_token_data;

    EnteredMacro(string macro, const std::vector<token_t> *tokens_):
                     name(macro), tokens(*tokens_) {}
    EnteredMacro(string macro, std::vector<token_t> &&tokens_):
                     name(macro), tokens(assembled_token_data),
                     assembled_token_data(std::move(tokens_)) {}
  };

  /**
  The basic C++ lexer. Extracts a single preprocessor token from the given reader.
  Uses @p cfile to include source metadata in the token.
  
  ISO C++ calls for nine phases of translation. The llreader passed to this call
  handles the first (file character mapping), and this routine handles the second
  and third. The data is not physically modified for any of these phases.
  */
  token_t read_token(llreader &cfile, error_handler *herr);

  /**
  Basic lexing/preprocessing unit; polled by all systems for tokens.
  
  This lexer calls out to `read_token` to handle phases 1-3 of translation.
  It then handles phases four (execution of preprocessing directives), five
  (expansion of string literals), and six (concatenation of adjacent string
  literals). This work needs to be handled when parsing C++11 or greater, as
  string literal contents can be used at compile time to have meaningful
  effect on output (this was also possible in previous versions, but through
  less conventional means that would not normally arise).
  
  Because of that nuance, and the general lookahead-heavy nature of parsing C++,
  token rewinding is a first-class feature of this lexer. When a string literal
  is encountered, another token is read immediately, and is either queued for
  return or concatenated to the current literal (depending on whether it is also
  a string literal). There is also a RAII helper, `look_ahead`, designed to
  facilitate handling of cases such as MVP. This way, a try-like branch of code
  can attempt to evaluate the tree one way, then seamlessly give up and allow
  a later branch to attempt the same.
  
  Thus, this lexer implementation has four layers of token source data:
    1. The open file stack. Files or string buffers (managed by an llreader) are
       lexed for raw tokens.
    2. Macros used within a file are expanded into tokens, and these buffers of
       tokens are stacked. Per ISO, a macro may not appear twice in this stack.
    3. Rewind operations produce queues of tokens. Each queue is stacked.
    4. During normal lexing operations, minor lookahead may be required.
       Tokens read during lookahead are queued at the top of this stack.d
  
  The above stacks are treated in stack order. They are generally populated in
  order from 1-4, but tokens are retrieved in order of 4-1.
  */
  class lexer {
    struct condition;

    llreader cfile;  ///< The current file being read.
    std::vector<openfile> files; ///< The files we have open, in the order we entered them.
    std::vector<EnteredMacro> open_macros; ///< Macros we are currently nested in.
    error_handler *herr;  ///< Error handler for problems during lex.

    /// Our conditional levels (one for each nested `\#if*`)
    vector<condition> conditionals;

    /// Tokens that have been expanded from a macro or fetched as lookahead.
    token_vector *buffered_tokens = nullptr;
    /// The position in the current token buffer.
    size_t buffer_pos;

    /// Buffer to which tokens will be recorded for later re-parse, as needed.
    token_vector *lookahead_buffer = nullptr;
    /// RAII type for initiating a lookahead.

    macro_map &macros; ///< Reference to the \c jdi::macro_map which will be used to store and retrieve macros.

    std::set<string> visited_files; ///< For record and reporting purposes only.

    /**
      Utility function designed to handle the preprocessor directive
      pointed to by \c pos upon invoking the function. Note that it should
      be the character directly after the pound pointed to upon invoking
      the function, not the pound itself.
      @param herr  The error handler to use if the preprocessor doesn't
                   exist or is malformed.
    **/
    void handle_preprocessor();

    /// Function used by the preprocessor to read in macro parameters in compliance with ISO.
    string read_preprocessor_args();
    /** Second-order utility function to skip lines until a preprocessor
        directive is encountered, then invoke the handler on the directive it found. **/
    void skip_to_macro();

    /// Enter a scalar macro, if it has any content.
    /// @param ms   The macro scalar to enter.
    void enter_macro(const macro_type &ms);
    /// Parse for parameters to a given macro function, if there are any, then evaluate
    /// the macro function and set the open file to reflect the change.
    /// This call should be made while the position is just after the macro name.
    /// @param mf   The macro function to parse
    /// @return Returns whether parameters were encountered and parsed.
    bool parse_macro_function(const macro_type &mf);
    /// Parse for parameters to a given macro function, if there are any.
    /// This call should be made while the position is just after the macro
    /// name. That is, the next token should be an opening parenthesis.
    /// If not, this method will return immediately.
    /// @param mf    The macro function to parse.
    /// @param dest  The vector to receive the individual parameters [out].
    /// @return Returns whether parameters were encountered and parsed.
    bool parse_macro_params(const macro_type &mf, vector<vector<token_t>>* dest);

    /// Pop the currently open file to return to the file that included it.
    /// @return Returns true if the buffer was successfully popped, and input remains.
    bool pop_file();

    /// Storage mechanism for conditionals, such as <code>\#if</code>, <code>\#ifdef</code>, and <code>\#ifndef</code>.
    struct condition {
      /// True if code in this layer is to be parsed
      /// (the condition that was given is true).
      bool is_true;
      /// True if an `else` statement or the like can set is_true to true.
      bool can_be_true;
      /// Convenience constructor.
      condition(bool, bool);
    };
    
   public:
    /// Read a raw token; this implies that TT_IDENTIFIER is the only token returned when any id is encountered: no keywords, no declarators, no definitions.
    token_t read_raw();
    /// Read a C++ token, with no scope information.
    token_t get_token();
    /// Read a C++ token, searching the given scope for names.
    token_t get_token_in_scope(definition_scope *scope);
    
    /// RAII type for initiating unbounded lookahead.
    class look_ahead {
      token_vector buffer;
      token_vector *prev_buffer;
      lexer *lex;

     public:
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
        if (prev_buffer) {
          if (prev_buffer->empty()) {
            prev_buffer->swap(buffer);
          } else {
            prev_buffer->insert(prev_buffer->end(), buffer.begin(), buffer.end());
          }
        }
      }
      void rewind();
    };

    // ============================================================================================
    // == Static Configuration Storage ============================================================
    // ============================================================================================
    
    /// Map of string to token type; a map-of-keywords type.
    typedef std::map<string,TOKEN_TYPE> keyword_map;
    /// List of C++ keywords, mapped to the type of their token.
    /// This list is assumed to contain tokens whose contents are unambiguous;
    /// one string maps to one token, and vice-versa.
    static keyword_map keywords;
    /// This is a map of macros to add bare-minimal support for a number of
    /// compiler-specific builtins.
    static macro_map kludge_map;

    /// Static cleanup function; safe to call without a matching init.
    static void cleanup();

    /** Consumes an llreader and attaches a new \c lex_macro.
        @param input    The file from which to read definitions.
                        This file will be manipulated by the system.
        @param pmacros  A \c jdi::macro_map which will receive
                        (and be probed for) macros.
        @param fname    The name of the file that was first opened.
    **/
    lexer(llreader& input, macro_map &pmacros, error_handler *herr);  // TODO: Have Lexer own pmacros.
    /**
      Consumes a token_vector, returning only the tokens in the vector before
      returning END_OF_CODE.
      @param input    The file from which to read definitions.
                      This file will be manipulated by the system.
      @param fname    The name of the file that was first opened.
    **/
    lexer(token_vector &&tokens, error_handler *herr);
    /** Destructor; free the attached macro lexer. **/
    ~lexer();
  };
}

#endif
