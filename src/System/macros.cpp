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
using namespace std;

#include "macros.h"
#include "../General/parse_basics.h"
using namespace jdip;

macro_type::macro_type(int ac): argc(ac) {}
macro_type::~macro_type() {}
  
macro_scalar::macro_scalar(string val): macro_type(-1), value(val) {}
macro_scalar::~macro_scalar() {}

macro_function::macro_function(string val): macro_type(0), value(), args() {
  value.push_back(val);
}
macro_function::macro_function(vector<string> arglist, string val, bool variadic): macro_type(arglist.size()+variadic), value(), args(arglist) {
  preparse_macro(value, args, val);
}
macro_function::~macro_function() {}

/**
  @section Implementation
  This function actually hauls quite a bit of ass. Its purpose is to make sure that
  when the parse is done, evaluating a macro function down the road is as simple as
  imploding the vector with a lookup on each element before concatenation.
  
  Since the function value parameter is read in raw, the function must handle both
  comments and strings, as well as, according to the expanded-value specification
  in \c jdip::macro_function::value, handle the # and ## operators.
**/
void jdip::preparse_macro(vector<string>& output, const vector<string>& params, string val)
{
  unsigned push_from = 0;
  map<string,int> parameters;
  
  output.clear(); // Wipe anything in the output array
  
  for (pt i = 0; i < params.size(); i++) // Load parameters into map for searching
    parameters[params[i]] = 0;
  
  for (pt i = 0; i < val.length(); ) // Iterate the string
  {
    if (val[i] == '\"') {       // *-*-*-*-*-* Skip strings *-*-*-*-*-* //
      while (val[++i] != '\"') // We're ultimately searching for the matching quote.
        if (val[i] == '\\')   // On the way, we may encounter escape sequences.
          i++; // But we do not need to handle them. Only '\\' and '\"' are of concern.
      continue;
    }
    if (val[i] == '\'')      {  // We handle single quotes the same way,
      while (val[++i] != '\'') // Looking for the terminating single quote,
        if (val[i] == '\\')   // Skipping a second character if we encounter a backslash.
          i++;
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
      if (parameters.find(val.substr(sp,i-sp)) != parameters.end()) // Then check if it's a parameter.
      {
        if (sp > push_from) // If we've covered any ground since our last output push,
          output.push_back(val.substr(push_from, sp-push_from)); // Push it onto our output
        output.push_back(val.substr(sp, i-sp)); // Push this identifier as a distinct item onto output
        push_from = i; // Indicate the new starting position of data to be pushed
      }
      continue;
    }
    
    if (val[i] == '#' and val[i+1] == '#')
    {
      pt ie = i++;
      while (ie > 0 and is_useless(val[ie-1])) --ie; // eliminate any leading whitespace
      if (ie > push_from) // If there's anything non-white to push since last time,
        output.push_back(val.substr(push_from, ie-push_from)); // Then push it
      while (is_useless(++i)); // Skip any trailing whitespace
    }
    
    i++;
  }
  output.push_back(val.substr(push_from));
}
