/**
 * @file value.h
 * @brief Header defining a structure for storing parsed values.
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

#ifndef _VALUE__H
#define _VALUE__H
#include <string>

namespace jdi {
  /**
    @enum VT
    Declares constants representing the possible types that \c value can hold.
  **/
  enum VT {
    VT_NONE, ///< No value has been assigned to this structure.
    VT_DOUBLE, ///< The value stored is a double.
    VT_INTEGER, ///< The value stored is an integer.
    VT_STRING ///< The value stored is a string.
  };

  /**
    @struct jdi::value
    A structure for storing and communicating data of varying types.
    This structure can contain any value defined in \enum VT.
  **/
  struct value {
    /** This union contains the actual data for each type.
    **/
    union {
      double d; ///< Any data stored as a floating point.
      long i; ///< Any data stored as an integer.
      const char* s; ///< Any data stored as a string. This value must be deleted if replaced or destructed.
    } val; ///< The value storage for this structure.
    
    VT type; ///< The type of our value. This little bugger is the reason we can't just be a big union.
    
    value(); ///< Construct a new, invalid value with no type (VT_NONE).
    value(double v); ///< Construct a new value representing a double.
    value(long v); ///< Construct a new value representing an integer.
    value(const char* v); ///< Construct a new value consuming and representing a const char*. Once you pass a const char*, it belongs to this class.
    value(std::string v); ///< Construct a new value representing a copy of the passed string. This operates in O(N).
    value(const value& v); ///< Copy a value. Handles allocation issues.
    ~value(); ///< Default destructor; handles freeing any strings.
  };
}

#endif
