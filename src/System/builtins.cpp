/**
 * @file  builtins.cpp
 * @brief A small source implementing the \c jdi::builtin context and functions
 *        for manipulating other builtins.
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

#include <map>
#include <string>
#include <vector>
#include <fstream>
using namespace std;

#include "builtins.h"

namespace jdip {
  map<string,USAGE_FLAG> builtin_flags;
}

using namespace jdip;
namespace jdi {
  jdi::context builtin(0);
  
  void read_declarators(const char* filename, USAGE_FLAG usage_flags)
  {
    string tname; // The name of this type, as it appears in code.
    ifstream in(filename);
    if (!in.is_open())
      return;
    while (!in.eof()) {
      in >> tname;
      add_declarator(tname, usage_flags);
    }
    in.close();
  }
  void add_declarator(string type_name, USAGE_FLAG usage_flags)
  {
    if (usage_flags & UF_FLAG) {
      pair<map<string,USAGE_FLAG>::iterator, bool> insit = builtin_flags.insert(pair<string,USAGE_FLAG>(type_name,usage_flags));
      insit.first->second = USAGE_FLAG(insit.first->second | usage_flags);
    }
  }
}
