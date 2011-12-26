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
  enum VT {
    VT_NONE,
    VT_DOUBLE,
    VT_INTEGER,
    VT_STRING
  };

  struct value {
    union {
      double d;
      long i;
      const char* s;
    } val;
    VT type;
    
    value();
    value(double v);
    value(long v);
    value(std::string v);
    value(const value& v);
    ~value();
  };
}

#endif
