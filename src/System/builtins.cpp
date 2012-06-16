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
  tf_map builtin_declarators;
  prim_map builtin_primitives;
  tf_flag_map builtin_decls_byflag;
  typeflag::typeflag(): usage(), flagbit(0), def(NULL) {}
  typeflag::typeflag(string n, USAGE_FLAG u): name(n), usage(u), flagbit(0), def(NULL) {}
  typeflag::~typeflag() { }
}

using namespace jdip;
namespace jdi {
  jdi::context builtin(0);
  
  unsigned long builtin_flag__volatile;
  unsigned long builtin_flag__static;
  unsigned long builtin_flag__const;
  unsigned long builtin_flag__register;
  unsigned long builtin_flag__inline;
  
  unsigned long builtin_flag__unsigned;
  unsigned long builtin_flag__signed;
  unsigned long builtin_flag__short;
  unsigned long builtin_flag__long;
  
  definition *builtin_type__unsigned;
  definition *builtin_type__signed;
  definition *builtin_type__short;
  definition *builtin_type__long;
  
  definition *builtin_type__void;
  definition *builtin_type__bool;
  definition *builtin_type__char;
  definition *builtin_type__int;
  definition *builtin_type__float;
  definition *builtin_type__double;
  
  definition *builtin_type__wchar_t;
  definition *builtin_type__va_list;
  
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
  add_decl_info add_declarator(string type_name, USAGE_FLAG usage_flags, string prim_name)
  {
    unsigned long flag;
    pair<tf_iter, bool> insit = builtin_declarators.insert(pair<string,typeflag*>(type_name,NULL));
    if (insit.second) {
      insit.first->second = new typeflag(type_name, usage_flags);
      if (usage_flags & (UF_PRIMITIVE | UF_STANDALONE)) {
        if (prim_name.empty())
          prim_name = type_name;
        pair<prim_iter, bool> ntit = builtin_primitives.insert(pair<string,definition*>(prim_name,NULL));
        if (ntit.second)
          ntit.first->second = new definition(prim_name, NULL, DEF_TYPENAME);
        insit.first->second->def = ntit.first->second;
      }
      if (usage_flags & UF_FLAG)
      {
        flag = 1 << builtin_decls_byflag.size();
        cout << type_name << "=>" << flag << endl;
        builtin_decls_byflag[flag] = insit.first->second;
        insit.first->second->flagbit = flag;
        if (type_name[0] != '_')
        {
          pair<tf_iter, bool> redit = builtin_declarators.insert(pair<string,typeflag*>("__" + type_name,NULL));
          if (redit.second) redit.first->second = new typeflag(*insit.first->second);
        }
      }
      else flag = 0;
    }
    else
      insit.first->second->usage = USAGE_FLAG(insit.first->second->usage | usage_flags), flag = 0;
    return add_decl_info(insit.first->second->def, flag);
  }
  
  add_decl_info::add_decl_info(definition *d, unsigned long f): def(d), flag(f) {}
  
  void add_gnu_declarators() {
    builtin_flag__volatile = add_declarator("volatile", UF_FLAG).flag;
    builtin_flag__static   = add_declarator("static",   UF_FLAG).flag;
    builtin_flag__const    = add_declarator("const",    UF_FLAG).flag;
    builtin_flag__register = add_declarator("register", UF_FLAG).flag;
    builtin_flag__inline   = add_declarator("inline",   UF_FLAG).flag;
    add_declarator("throw", UF_FLAG);
    
    jdi::add_decl_info
    c = add_declarator("unsigned", UF_STANDALONE_FLAG, "int");
    builtin_type__unsigned = c.def, builtin_flag__unsigned = c.flag;
    c = add_declarator("signed",   UF_STANDALONE_FLAG, "int");
    builtin_type__signed   = c.def, builtin_flag__signed   = c.flag;
    c = add_declarator("short",    UF_PRIMITIVE_FLAG);
    builtin_type__short    = c.def, builtin_flag__short    = c.flag;
    c = add_declarator("long",     UF_PRIMITIVE_FLAG);
    builtin_type__long     = c.def, builtin_flag__long     = c.flag;
    
    builtin_type__void   = add_declarator("void",    UF_PRIMITIVE).def;
    builtin_type__bool   = add_declarator("bool",    UF_PRIMITIVE).def;
    builtin_type__char   = add_declarator("char",    UF_PRIMITIVE).def;
    builtin_type__int    = add_declarator("int",     UF_PRIMITIVE).def;
    builtin_type__float  = add_declarator("float",   UF_PRIMITIVE).def;
    builtin_type__double = add_declarator("double",  UF_PRIMITIVE).def;
    
    builtin_type__wchar_t = add_declarator("wchar_t",   UF_PRIMITIVE).def;
    builtin_type__va_list = add_declarator("__builtin_va_list",   UF_PRIMITIVE).def;
    
    builtin.variadics.insert(builtin_type__va_list);
  }
  
  void cleanup_declarators() {
    for (tf_iter tfit = builtin_declarators.begin(); tfit != builtin_declarators.end(); tfit++)
      delete tfit->second;
    for (prim_iter prit = builtin_primitives.begin(); prit != builtin_primitives.end(); prit++)
      delete prit->second;
  }
}
