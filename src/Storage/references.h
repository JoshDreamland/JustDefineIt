/**
 * @file  references.h
 * @brief System header declaring a structure for working with C referencers.
 * 
 * The asterisk pointer symbol (*), ampersand reference symbol (&), bracket
 * array bound indicators ([]), and function parameter parentheses ((*)())
 * are each unique types of references dealt with in this file.
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

#ifndef _REFERENCES__H
#define _REFERENCES__H

namespace jdi {
  /**
    @struct jdi::ref_stack
    @brief  A stack-like structure representing C referencers.
    
    A reference stack is an info stack stored with any typed definition.
    The stack can depict any of C's standard referencers; the asterisk
    pointer symbol (*), the ampersand reference symbol (&), bracket array
    bound indicators ([]), and function parameter parentheses ((*)()).
    
    The stack is ordered such that each dereference symbol (the asterisk
    symbol or a set of array bounds, or a list of function parameters)
    is evaluated simply by popping one item from a copy of the stack.
  **/
  struct ref_stack {
    /// Clear the stack, undoing all referencers.
    void clear();
  };
}

#endif
