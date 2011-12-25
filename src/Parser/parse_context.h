/**
 * @file  parse_context.h
 * @brief A file defining variables needed by the parser as it runs--Not to be
 *        confused with context.h!
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

#ifndef _PARSE_CONTEXT__H
#define _PARSE_CONTEXT__H

#include "../General/quickstack.h"
#include "../General/llreader.h"
#include "../Storage/definition.h"
namespace jdip {
  using quickstack::stack;
  using namespace jdi;
  
  /**
    @struct jdip::parse_context
    @brief  A structure storing info needed by the parser as it runs.
    
    This structure contains any information which cannot be discerned based on
    a position in a handler function. For instance, this file contains information
    regarding what source file is currently being read and which source file it came from.
  **/
  struct parse_context {
    definition* global; ///< The global scope in this context
    stack<llreader> files; ///< The files we have open
    string error; ///< Any error text
    string err_file; ///< The file in which an error occurred
    int err_line; ///< The line number on which the error occurred
    int err_pos; ///< The position at which the error occurred
    /// Default constructor, taking a pointer to the enclosing context.
    parse_context();
  };
}

#endif
