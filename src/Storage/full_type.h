/**
 * @file  full_type.h
 * @brief Header declaring a type storing a \c definition, \c ref_stack, and flag word.
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

#ifndef _FULL_TYPE__H
#define _FULL_TYPE__H

namespace jdi {
  struct full_type;
}

#include <Storage/definition.h>
#include <Storage/references.h>

namespace jdi {
  /**
    @struct jdi::full_type
    A structure noting the \c definition associated with a type along with a
    set of modifier flags.
  **/
  struct full_type {
    jdi::definition *def; ///< The \c definition associated with the type
    jdi::ref_stack refs; ///< Any referencers affecting this type, such as the pointer-to asterisk (*) or ampersand reference (&).
    int flags; ///< Any flags, such as unsigned, signed, or const, associated with us, as a bitmask.
    
    void swap(full_type& ft); ///< Trade contents with another full_type.
    
    full_type(); ///< Default constructor.
    full_type(jdi::definition*); ///< Construct with only a definition.
    full_type(jdi::definition*, jdi::ref_stack, int); ///< Construct from individual components. Consumes the given \c ref_stack.
    full_type(const full_type&); ///< Copy constructor. Makes a copy, so slowish.
  };
}

#endif