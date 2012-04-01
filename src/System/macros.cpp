/**
 * @file  macros.cpp
 * @brief System source implementing functions for parsing macros.
 * 
 * In general, the implementation is unremarkable.
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

#include <map>
#include <string>
#include <vector>
#include <cstring>
using namespace std;

#include "macros.h"
#include <General/parse_basics.h>
#include <General/debug_macros.h>
using namespace jdip;

macro_type::macro_type(string n, int ac): argc(ac), refc(1), name(n) {}
macro_type::~macro_type() {}

macro_scalar::macro_scalar(string n, string val): macro_type(n,-1), value(val) {}
macro_scalar::~macro_scalar() {}

macro_function::macro_function(string n, string val): macro_type(n,0), value(), args() {
  register const size_t bs = val.length();
  char *buf = new char[bs]; memcpy(buf, val.c_str(), bs);
  value.push_back(mv_chunk(buf, bs));
}
macro_function::macro_function(string n, const vector<string> &arglist, string val, bool variadic): macro_type(n,arglist.size()+variadic), value(), args(arglist) {
  preparse(val);
}
macro_function::~macro_function() {
  for (size_t i = 0; i < value.size(); ++i)
    if (!value[i].is_arg) delete []value[i].data;
}

void macro_type::free(const macro_type* whom) {
  if (!-- whom->refc){
    if (whom->argc >= 0) delete (macro_function*)whom;
    else delete (macro_scalar*)whom;
  }
}


//======================================================================================================
//=====: Macro function data chunk constructors :=======================================================
//======================================================================================================

macro_function::mv_chunk::mv_chunk(const char* str, size_t start, size_t len): metric(len), is_arg(false) {
  data = new char[len];
  memcpy(data, str+start, len);
}
macro_function::mv_chunk::mv_chunk(char* ndata, size_t len): data(ndata), metric(len), is_arg(false) {}
macro_function::mv_chunk::mv_chunk(size_t argnum): data(NULL), metric(argnum), is_arg(true) {}

string macro_function::mv_chunk::toString() {
  if (is_arg) return "arg";
  return string(data,metric);
}

//======================================================================================================
//=====: Macro function parsing methods :===============================================================
//======================================================================================================

/**
  @section Implementation
  This function actually hauls quite a bit of ass. Its purpose is to make sure that
  when the parse is done, evaluating a macro function down the road is as simple as
  imploding the vector with a lookup on each element before concatenation.
  
  Since the function value parameter is read in raw, the function must handle both
  comments and strings, as well as, according to the expanded-value specification
  in \c jdip::macro_function::value, handle the # and ## operators.
**/
void jdip::macro_function::preparse(string val)
{
  unsigned push_from = 0;
  map<string,int> parameters;
  
  value.clear(); // Wipe anything in the output array
  
  for (pt i = 0; i < args.size(); i++) // Load parameters into map for searching and fetching index
    parameters[args[i]] = i;
  
  for (pt i = 0; i < val.length(); ) // Iterate the string
  {
    if (val[i] == '\"' or val[i] == '\'') { // Skip strings
      register char nc = val[i];
      while (++i < val.length() and val[i] != nc) // We're ultimately searching for the matching quote.
        if (val[i] == '\\') ++i; // No need to handle escape sequences; only '\\' and '\"' are of concern.
      continue;
    }
    
    if (val[i] == '/') { // Unfortunately, we must also do comment checking.
      if (val[i+1] == '*') { i += 3; // This is a hack to save lines on this trivialty
        while (i < val.length() and (val[i] != '/' or val[i-1] != '*')) ++i;
      } else if (val[i+1] == '/')
        while (++i < val.length() and val[i] != '\n');        
    }
    
    if (is_digit(val[i])) { // We must also be sure to skip numbers,
      while (is_digit(val[++i])); // As they could have postfixes matching a parameter
      continue;
    }
    
    if (is_letter(val[i])) // If we do encounter a letter, the fun begins.
    {
      const unsigned sp = i; // We record where we're leaving off
      while (is_letterd(val[++i])); // And navigate to the end of the identifier.
      map<string,int>::iterator pi = parameters.find(val.substr(sp,i-sp));
      if (pi != parameters.end()) // Then check if it's a parameter.
      {
        if (sp > push_from) // If we've covered any ground since our last value push,
          value.push_back(mv_chunk(val.c_str(), push_from, sp-push_from)); // Push it onto our value
        value.push_back(mv_chunk(pi->second)); // Push this identifier as a distinct item onto value
        push_from = i; // Indicate the new starting position of data to be pushed
      }
      continue;
    }
    
    if (val[i] == '#')
    {
      if (val[++i] == '#') {
        if (i <= 1) continue;
        pt ie = i - 2;
        while (ie > 0 and is_useless(val[ie-1])) --ie; // eliminate any leading whitespace
        if (ie > push_from) // If there's anything non-white to push since last time,
          value.push_back(mv_chunk(val.c_str(), push_from, ie-push_from)); // Then push it
        while (is_useless(val[++i])); // Skip any trailing whitespace
        push_from = i;
        continue;
      }
    }
    
    i++;
  }
  value.push_back(mv_chunk(val.c_str(), push_from, val.length() - push_from));
}

#include <iostream>
#include <System/token.h>
bool macro_function::parse(const vector<string> &arg_list, llreader &dest, error_handler *herr, token_t errtok)
{
  if (arg_list.size() < args.size())
    return errtok.report_error(herr, "Too few arguments to macro function `" + name + "': provided " + toString(arg_list.size()) + ", requested " + toString(args.size())), false;
  if ((arg_list.size() > args.size() and args.size() == (unsigned)argc))
    return errtok.report_error(herr, "Too many arguments to macro function `" + name + "'"), false;
  size_t alloc = 1;
  for (size_t i = 0; i < value.size(); i++) {
    if (value[i].is_arg)
      alloc += arg_list[value[i].metric].length();
    else {
      if (*value[i].data == '#') {
        dbg_assert(value[i].metric == 1);
        alloc += arg_list[value[++i].metric].length() << 1;
        continue;
      }
      alloc += value[i].metric;
    }
  }
  
  bool lwa = false; // "Last was arg"--True if the previous chunk was an argument.
  char* buf = new char[alloc], *bufat = buf;
  for (size_t i = 0; i < value.size(); i++)
    if (value[i].is_arg) {
      if (lwa) {
        while (bufat > buf and is_useless(*--bufat)); ++bufat;
        const string &ts = arg_list[value[i].metric];
        register const char* argname = ts.c_str();
        register size_t sz = ts.length();
        while (is_useless(*argname)) ++argname, --sz;
        memcpy(bufat, argname, sz);
        bufat += sz;
      }
      else {
        register const string& argname = arg_list[value[i].metric];
        memcpy(bufat, argname.c_str(), argname.length());
        bufat += argname.length();
        lwa = false;
      }
      lwa = true;
    }
    else {
      memcpy(bufat, value[i].data, value[i].metric);
      bufat += value[i].metric;
    }
  *bufat = 0;
  dest.consume(buf, bufat-buf);
  return true;
}
