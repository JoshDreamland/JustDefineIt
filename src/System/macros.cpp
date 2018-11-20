/**
 * @file  macros.cpp
 * @brief System source implementing functions for parsing macros.
 * 
 * In general, the implementation is unremarkable.
 * 
 * @section License
 * 
 * Copyright (C) 2011-2012 Josh Ventura
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

#include <map>
#include <string>
#include <vector>
#include <cstring>
using namespace std;

#include "macros.h"
#include <General/parse_basics.h>
#include <General/debug_macros.h>
#include <System/lex_cpp.h>
using namespace jdip;

macro_type::macro_type(const string &n, vector<token_t> &&val):
    is_function(false), is_variadic(false), name(n), args(),
    value(std::move(val)) {}
macro_type::macro_type(string_view n, vector<string> &&arglist,
                       vector<token_t> &&val, bool variadic):
    is_function(true), is_variadic(variadic), name(n), args(std::move(arglist)),
    value(std::move(val)) {}
macro_type::~macro_type() {}

void macro_type::free(const macro_type* whom) {
  delete whom;
}


//==============================================================================
//=====: Macro function parsing methods :=======================================
//==============================================================================


string macro_type::toString() const {
  string res = "#define " + name;
  if (is_function) {
    res += "(";
    for (size_t i = 0; i < args.size(); i++)
      res += args[i]
          + (i + 1 < args.size() ? ", " : is_variadic ? "..." : "");
    res += ") \\\n";
  } else {
    res += " \\\n";
  }
  for (size_t i = 0; i < value.size(); ++i)
    res += "  " + value[i].to_string() + (i+1 < value.size()? "\\\n" : "");
  return res;
}

