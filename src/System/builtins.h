/**
 * @file  builtins.h
 * @brief A header declaring a centralized context representing built-in objects.
 * 
 * This file is used for configuring the parse environment ahead of time. Contents
 * of the builtin context are inherited by user-created contexts, and the contents
 * of \c builtin_flags determines accepted modifiers when parsing code in general,
 * regardless of the enclosing context.
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


#ifndef _JDI_BUILTINS__H
#define _JDI_BUILTINS__H

#include <API/context.h>

namespace jdip {
  using namespace jdi;
  /**
    A map of flags to their usage.
    This map *only* contains flags. Types can be aliased through typedef,
    and as such must be represented in a /c jdi::context.
    @see jdi::USAGE_FLAG
    @see jdi::builtin
  **/
  extern map<string,USAGE_FLAG> builtin_flags;
}

namespace jdi {
  /**
    A context containing all built-in objects and definitions.
    This context contains all types, macros, and search directories that are
    considered to be built in to the compiler. Here, you will find types such
    as int and double, macros such as __WIN32__ or __GNUG__, as well as search
    directories such as /usr/include.
  **/
  extern jdi::context builtin;
  
  /**
    Read a list of declarators from a file, toggling on their usage with the given flags.
    @param filename      The name of the file from which to read the declarators.
    @param usage_flags   The usage flag to be enabled for the read declarators.
    @see \c jdi::USAGE_FLAG
  **/
  void read_declarators(const char* filename, USAGE_FLAG usage_flags);
  /**
    Add a single declarator, toggling on its usage for the given flag.
    @param type_name     The name of the declarator, as it appears in the code.
    @param usage_flags   The usage flag to be enabled for the given declarator.
    @see \c jdi::USAGE_FLAG
  **/
  void add_declarator(string type_name, USAGE_FLAG usage_flags);
}

#endif
