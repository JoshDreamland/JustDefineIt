/**
 * @file  macros.h
 * @brief System header for parsing macros.
 * 
 * This header defines a few classes for representing macros in memory, both
 * macro functions and simple preprocessor substitutions.
 * 
 * Some of the constructors in this file do some heavy lifting to make the general
 * interface more pleasant in parser functions, as well as ultimately to make the
 * code more efficient.
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

#ifndef _MACROS__H
#define _MACROS__H

namespace jdi {
  struct macro_type;
}

#include <map>
#include <string>
#include <string_view>
#include <vector>
#include <General/llreader.h>
#include <API/error_reporting.h>
#include <System/token.h>

namespace jdi {
  using std::string;
  using std::string_view;
  using std::vector;
  
  /**
    @struct macro_type
    @brief  A generic structure representing a macro in memory.
    
    This is the base class beneath \c macro_scalar and \c macro_function. It should not
    be instantiated purely, as it does not contain any accessible value; its only purpose
    is for storing different types of macros together. \
  **/
  struct macro_type {
    /** 
      Argument count, or -1 if not a function.
      Hence, a value of -1 implies that this is an instance of macro_scalar.
      Otherwise, this is an instance of macro_function.
      If this value is greater than the size of the argument vector in the macro_function, then the function is variadic.
    **/
    const bool is_function, is_variadic;
    /// A copy of the name of this macro.
    string name;
    /// A copy of the parameter list of this macro.
    vector<string> args;
    /// Copy of the string from which this macro was parsed. Referenced by `value`.
    string raw_value;
    
    /// The definiens of this macro, as a series of preprocessor tokens.
    vector<token_t> value;
    
    /// Release a macro
    static void free(const macro_type* whom);
    
    /// Convert this macro to a string
    string toString() const;
    
    ~macro_type();
      
    /// Default constructor; construct a zero-parameter macro function with the given value.
    macro_type(const string &n, vector<token_t> &&definiens);
    /** Construct a macro function taking the arguments in arg_list.
        This function parses the given value based on the argument list.
        @param name      The name of this macro.
        @param arg_list  Contains the arguments to be copied in.
        @param value     The value that will be assigned to this macro function. 
                         The constructor will automatically parse and expand
                         this value according to convention.
        @param variadic  Determines if an additional parameter should be created
                         to store all excess arguments.
        @param herr      The error handler to receive any errors.
        @note
          If \p arg_list is empty, and \p variadic is false, the behavior is the same as the default constructor. 
    **/
    macro_type(string_view name, vector<string> &&arg_list,
               vector<token_t> &&definiens, bool variadic = false);
  };
  
  typedef std::map<string, const jdi::macro_type*> macro_map; ///< Map type used for storing macros
  typedef macro_map::iterator macro_iter; ///< Iterator type for macro maps.
  typedef macro_map::const_iterator macro_iter_c; ///< Const iterator type for macro maps.
}
#endif
