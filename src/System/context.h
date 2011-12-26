/**
 * @file context.h
 * @brief Header containing methods for creating contexts of parsed code.
 * 
 * One idea that surfaced which seemed like a keeper was a system for creating
 * transferrable contexts. What we mean by that is allowing users to decide how
 * many parse contexts to create and what to parse with them, and providing a set
 * of functions to copy definitions from one context to another. This can be done
 * by class or namespace, or by copying the entire context at once. This enables
 * users to optimize their use of the parser. For instance, by parsing large libs
 * such as windows.h or opengl.h as individual contexts, users are then able to
 * parse smaller headers which include or otherwise depend on them by creating a
 * new context, copying the contents of the large header contexts over, and then
 * invoking the parser on the remaining code.
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

#ifndef _JDI_CONTEXT__H
#define _JDI_CONTEXT__H

#include <map>
#include <string>
#include <vector>
#include <iostream>

#include "macros.h"
#include "type_usage_flags.h"
#include "../System/type_usage_flags.h"
#include "../Storage/definition.h"
#include "../General/llreader.h"
#include "../Parser/parse_context.h"

namespace jdip { class context_parser; }

namespace jdi
{
  using std::map;
  using std::string;
  using std::vector;
  using std::ostream;
  using std::cout;
  
  typedef map<string,jdip::macro_type*> macro_map;
  typedef macro_map::iterator macro_iter;
  
  /**
    @class context
    @brief A class representing a context for manipulation.
    
    The context class allows users to create, search, manage, and splice collections
    of parsed source code on separate bases.
    
    @see context.h
  **/
  
  class context
  {
    friend class jdip::context_parser; ///< Make sure the parser can read this junk
    macro_map macros; ///< A map of macros defined in this context.
    vector<string> search_directories; ///< A list of #include directories in the order they will be searched.
    definition_scope* global; ///< The global scope represented in this context.
    jdip::parse_context* pc; ///< Any open parse context in this definition context.
    
    public:
    
    /** Add a type name to this context
        The type will be added as a primitive. To add a typedef, use \c jdi::context::add_typedef().
        @param name The name of the type, as it appears in code.
    **/
    void add_typename(string name);
    /** Add a typedef'd type to this context.
        This function creates a new type aliased to an existing type. Hence, \p definiens
        must not be NULL. Chances are, if you don't have anything to pass to definiens, you
        are wanting to create a primitive type. See \c jdi::context::add_typename().
        @param definiendum  The name of the aliased type, as it appears in code.
        @param definiens    The type to which the created type will be aliased. This cannot be NULL.
    **/
    void add_typedef(string definiendum, definition *definiens);
    
    void read_macros(const char* filename); ///< Read a file containing exclusively macros, in C format.
    void add_macro(string definiendum, string definiens); ///< Add a macro to this context.
    
    /// Add a macro function with no parameters to this context.
    void add_macro_func(string definiendum, string definiens);
    /// Add a macro function with one parameter to this context.
    /// If variadic is true, the parameter can contain commas.
    void add_macro_func(string definiendum, string p1, string definiens, bool variadic);
    /// Add a macro function with two parameters to this context.
    /// If variadic is true, the second parameter can contain commas.
    void add_macro_func(string definiendum, string p1, string p2, string definiens, bool variadic); 
    /// Add a macro function with three parameters to this context.
    /// If variadic is true, the third parameter can contain commas.
    void add_macro_func(string definiendum, string p1, string p2, string p3, string definiens, bool variadic); 
    
    /// Read a file containing a list of #include search directories, one full path per line.
    void read_search_directories(const char* filename); 
    /// Same as read_search_directories, but designed to parse GNU cpp output.
    /// Ignores all lines up to and including the first line containing begin_line,
    /// then reads in all additional lines as search directories until it reaches end_line.
    void read_search_directories_gnu(const char* filename, const char* begin_line, const char* end_line);
    /// Add an #include search directory to this context.
    void add_search_directory(string dir);
    
    void reset(); ///< Reset back to the built-ins; delete all parsed definitions
    void reset_all(); ///< Reset everything, dumping all built-ins as well as all parsed definitions
    
    /** Load standard built-in types, such as int. 
        This function is really only for use with the built-in context.
    **/
    void load_standard_builtins(); 
    /** Load GNU-specific built-in types, such as nonsense_t.
        This function is really only for use with the built-in context.
    **/
    void load_gnu_builtins();
    
    void output_types(ostream &out = cout); ///< Print a list of scoped-in types.
    void output_macros(ostream &out = cout); ///< Print a list of scoped-in macros.
    void output_definitions(ostream &out = cout); ///< Print a any scoped-in other shit.
    
    /** Parse an input stream for definitions.
        @param cfile    The stream to be read in.
    **/
    int parse_C_stream(llreader& cfile);
    
    /** Retrieve the last error message, if any error was thrown during parse.
        @return The last error message, or an empty string if no error occurred.
    **/
    string get_last_error();
    
    /** Default constructor; allocates a global context with built-in definitions.
        Definitions are copied into the new context from the \c builtin context.
    **/
    context();
    
    /** Integer constructor. This constructor circumvents the copy process. It has
        no purpose other than and is not to be used except in allocating the builtin scope.
        While it is not necessary under common circumstances to avoid copying from the
        global scope into itself on construct, should the scope ever be populated before
        the ctor body due to one change or another, it could potentially cause issues.
        
        As the issues are as unlikely as the scenario, this mehtod may eventually be removed.
        For the time being, we may as well keep a distinction between the constructors of the
        normal scopes and of the builtin scope.
        
        @param disregarded  Disregarded. The parameter is there only to distinguish this constructor.
    **/
    context(int disregarded);
    
    /** Copy constructor.
        Overrides the C++ default copy constructor with a version meant to simplify
        building off of existing contexts. Simply constructs and duplicates the passed
        context instead of the builtin context.
    **/
    context(const context&);
    
    /** A simple destructor to clean up after the definition loading. **/
    ~context();
  };
}

#endif
