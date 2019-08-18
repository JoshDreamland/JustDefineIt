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
using namespace jdi;

macro_type::macro_type(const string &n, vector<token_t> &&val):
    is_function(false), is_variadic(false), name(n), params(),
    value(std::move(val)) {}
macro_type::macro_type(string_view n, vector<string> &&arglist,
                       vector<token_t> &&val, bool variadic):
    is_function(true), is_variadic(variadic), name(n),
    params(std::move(arglist)), value(std::move(val)) {}
macro_type::~macro_type() {}

//==============================================================================
//=====: Macro function parsing methods :=======================================
//==============================================================================

string macro_type::toString() const {
  string res = "#define " + name;
  if (is_function) {
    res += "(";
    for (size_t i = 0; i < params.size(); i++)
      res += params[i]
          + (i + 1 < params.size() ? ", " : is_variadic ? "..." : "");
    res += ") \\\n";
  } else {
    res += " \\\n";
  }
  for (size_t i = 0; i < value.size(); ++i)
    res += "  " + value[i].to_string() + (i + 1 < value.size()? "\\\n" : "");
  return res;
}

static token_t paste_tokens(token_t left, token_t right, error_handler *herr) {
  string buf = left.content.toString() + right.content.toString();
  llreader read("token concatenation", buf, false);
  token_t res = read_token(read, herr);
  if (!read.eof()) {
    herr->error(left, "Concatenation of `%s` and `%s` does not yield a coherent"
                " token.", left.to_string(), right.to_string());
  }
  return res;
}

static void append_or_paste(token_vector &dest,
                            token_vector::const_iterator begin,
                            token_vector::const_iterator end,
                            bool paste, error_handler *herr) {
  if (begin == end) return;
  if (paste) {
    token_t &left = dest.back();
    left = paste_tokens(left, *begin++, herr);
  }
  dest.insert(dest.end(), begin, end);
}

token_vector macro_type::substitute_and_unroll(
    const vector<token_vector> &args, error_handler *herr) const {
  token_vector res;
  bool paste_next = false;
  for (const FuncComponent &part : parts) {
    switch (part.tag) {
      case FuncComponent::TOKEN_SPAN:
        append_or_paste(res, value.begin() + part.token_span.begin,
                             value.begin() + part.token_span.end,
                        paste_next, herr);
        break;
      case FuncComponent::ARGUMENT:
        append_or_paste(res, args[part.argument.index].begin(),
                             args[part.argument.index].end(),
                        paste_next, herr);
        break;
      case FuncComponent::PASTE:
        paste_next = true;
        break;
      default:
        herr->error("Internal error: Macro function component unknown...");
    }
  }
  return res;
}
