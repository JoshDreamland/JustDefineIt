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

#include <string>
#include <vector>

namespace jdip {
  using std::string;
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
    **/
    const int argc;
    /// Macros are very pliable; this tells how many references are made to this macro.
    mutable unsigned refc;
    protected:
      /**
        The macro_type constructor is used to set the argument count or flag it inactive.
        As macro_type should never be instantiated purely, its constructor is protected. Modifying
        the parameter count could cause failure to recognize the type of the current macro.
        @see argc
      **/
      macro_type(int argc);
      /// The base destructor of macro_type does not do anything. DON'T INVOKE IT!
      ~macro_type();
  };
  
  /**
    @struct macro_scalar
    @brief  A structure representing a macro substitution in memory.
  **/
  struct macro_scalar: macro_type {
    string value; ///< The definiens of this macro.
    macro_scalar(string val=""); ///< The default constructor, taking an optional value parameter.
    ~macro_scalar(); ///< The macro_scalar destructor is only for debugging purposes.
  };
  
  /**
    @struct macro_function
    @brief  A structure representing a macro function in memory.
  **/
  struct macro_function: macro_type {
    /** The expanded value of this macro.
        
        @section Convention
        
        The definiens of this macro must be stored expanded such that each used parameter is given its own element.
        For example, given #define vertex_x(a,b,c) ((-(b))/(2*(a))), the value member will be expanded as follows:
        
          {   "((-(",   "b",   "))/(2*(",   "a",   ")))"   }.
        
        Use of the concatenation operator, "##", will be automatically accounted for, such that the definiens "a##b=c"
        will be expanded as such:
        
          {   "a",    "b",    "=",   "c"   }
        
        The string operator, #, will be handled by appending the singleton string "#" to the vector, followed by
        the name of the parameter. This is the only special case which the imploding function must implement.
        
        By this convention, evaluating a macro function is as simple as unloading the argument-value pairs into a
        map and iterating the value vector, substituting value[i] with map[value[i]] where defined.
    **/
    vector<string> value;
    vector<string> args; //!< The names of each argument.
    
    /// Construct a zero-parameter macro function with the given value, or an empty value if none is specified.
    macro_function(string val="");
    /** Construct a macro function taking the arguments in arg_list.
        This function parses the given value based on the argument list.
        @param arg_list  Contains the arguments to be copied in.
        @param value     The value that will be assigned to this macro function. 
                         The constructor will automatically parse and expand this value according to convention.
        @param variadic  Determines if an additional parameter should be created to store all excess arguments.
        @note
          If \p arg_list is empty, and \p variadic is false, the behavior is the same as the default constructor. 
    **/
    macro_function(vector<string> arg_list, string value="", bool variadic=0);
    /// Big surprise: The macro_function destructor also does nothing.
    ~macro_function();
  };
  
  /** An internal function used to parse the definiens of a macro into a vector for collapse at eval-time.
      Saves big on CPU when evaluating a function many times.
      @see jdi::macro_function::value
  **/
  void preparse_macro(vector<string>& output, const vector<string>& params, string definiens);
}
#endif
