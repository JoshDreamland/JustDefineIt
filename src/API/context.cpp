/**
 * @file  context.cpp
 * @brief Source implementing methods for creating contexts of parsed code.
 * 
 * In general, the implementation is unremarkable. See the header documentation
 * for details on behavior and usage.
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

#include <string>
#include <fstream>
#include <cstring>
using namespace std;

#include "context.h"
#include <System/macros.h>
#include <System/builtins.h>

using namespace jdi;
using namespace jdip;

void context::read_macros(const char* filename)
{
  ifstream in(filename);
  if (!in.is_open())
    return;
  // TODO: IMPLEMENT
  in.close();
}
void context::add_macro(string definiendum, string definiens) {
  macros[definiendum] = (macro_type*)new macro_scalar(definiens);
}
void context::add_macro_func(string definiendum, string definiens) {
  macros[definiendum] = (macro_type*)new macro_function(definiens);
}
void context::add_macro_func(string definiendum, string p1, string definiens, bool variadic)
{
  vector<string> arglist;
  arglist.push_back(p1);
  macros[definiendum] = (macro_type*)new macro_function(arglist, definiens, variadic);
}
void context::add_macro_func(string definiendum, string p1, string p2, string definiens, bool variadic)
{
  vector<string> arglist;
  arglist.push_back(p1);
  arglist.push_back(p2);
  macros[definiendum] = (macro_type*)new macro_function(arglist, definiens, variadic);
}
void context::add_macro_func(string definiendum, string p1, string p2, string p3, string definiens, bool variadic)
{
  vector<string> arglist;
  arglist.push_back(p1);
  arglist.push_back(p2);
  arglist.push_back(p3);
  macros[definiendum] = (macro_type*)new macro_function(arglist, definiens, variadic);
}

#ifndef MAX_PATH
#define MAX_PATH 512
#endif

void context::read_search_directories(const char* filename) {
  read_search_directories_gnu(filename,NULL,NULL);
}
void context::read_search_directories_gnu(const char* filename, const char* begin_line, const char* end_line)
{
  ifstream in(filename);
  if (!in.is_open()) return;
  
  const size_t ln = strlen(filename) + MAX_PATH;
  const size_t bll = begin_line ? strlen(begin_line) : size_t(-1), ell = end_line ? strlen(end_line) : size_t(-1);
  char *sdir = new char[ln];
  
  while (!in.eof())
  {
    *sdir = 0;
    in.getline(sdir,ln);
    const size_t sl = strlen(sdir);
    if (sl < bll)
      continue;
    if (strncmp(sdir + sl - bll, begin_line, bll) == 0)
      break;
  }
  while (!in.eof())
  {
    *sdir = 0;
    in.getline(sdir,ln);
    string dir = sdir;
    if (dir.length() >= ell)
      if (strncmp(sdir, end_line, ell) == 0)
        break;
    add_search_directory(dir);
  }
  
  delete[] sdir;
  in.close();
}
void context::add_search_directory(string dir)
{
  search_directories.push_back(dir);
}

void context::reset()
{
  
}
void context::reset_all()
{
  
}

string context::get_last_error() {
  return error;
}

void context::load_standard_builtins()
{
  // Nothing to load
}
void context::load_gnu_builtins()
{
  
}

#include <iostream>

void context::output_types(ostream &out) {
  out << "Unimplemented";
}
void context::output_macros(ostream &out) 
{
  for (macro_iter it = macros.begin(); it != macros.end(); it++)
  {
    if (it->second->argc >= 0)
    {
      macro_function *mf = (macro_function*)it->second;
      out << "#define " << it->first << "(";
      for (size_t i = 0; i < mf->args.size(); i++)
        out << mf->args[i] << (i+1 < mf->args.size() ? ", " : ((size_t)it->second->argc > mf->args.size()? "...": ""));
      out << ") \\" << endl;
      for (size_t i = 0; i < mf->value.size(); i++)
        out << "  " << mf->value[i] << (i+1 < mf->value.size()? "\\" : "") << endl;
    }
    else {
      out << "#define " << it->first << endl << "  " << ((macro_scalar*)it->second)->value << endl;
    }
  }
}

static void utility_printrc(definition_scope* scope, ostream &out, string indent) {
  for (map<string,definition*>::iterator it = scope->members.begin(); it != scope->members.end(); it++)
  {
    out << indent;
    if (it->second->flags & DEF_TYPED)
    {
      for (int i = 1; i <= 0x10000; i <<= 1)
        if (((definition_typed*)it->second)->flags & i)
          out << builtin_decls_byflag[i]->name << " ";
      if (((definition_typed*)it->second)->type)
        out << ((definition_typed*)it->second)->type->name << " ";
      else out << "<NULL> ";
      out << it->second->name;
      out << ";" << endl;
    }
    else if (it->second->flags & DEF_NAMESPACE)
    {
      out << indent << "namespace " << it->second->name << " {" << endl;
      utility_printrc((definition_scope*)it->second, out, indent + "  ");
      out << indent << "}" << endl;
    }
    else if (it->second->flags & DEF_CLASS)
    {
      out << "class " << it->second->name;
      definition_class *dc = (definition_class*)it->second;
      if (dc->ancestors.size()) {
        out << ": ";
        for (size_t i = 0; i < dc->ancestors.size(); i++)
          out << (dc->ancestors[i].protection == DEF_PRIVATE? "private "
                 :dc->ancestors[i].protection == DEF_PROTECTED? "protected "
                 :"public ") << dc->ancestors[i].def->name
              << (i+1 < dc->ancestors.size() ? ", " : "");
      }
      out << " {" << endl;
      utility_printrc((definition_scope*)it->second, out, indent + "  ");
      out << indent << "}" << endl;
    }
  }
}
void context::output_definitions(ostream &out) {
  utility_printrc(global, out, "");
}

context::context(): global(new definition_scope()), pc(NULL) {
  builtin.global->copy(global);
}

context::context(int): global(new definition_scope()), pc(NULL) { }

context::~context() {
  delete global;
  
  // Clean up macros
  for (macro_iter it = macros.begin(); it != macros.end(); it++) {
    if (it->second->argc >= 0)
      delete (macro_function*)it->second;
    else
      delete (macro_scalar*)it->second;
  }
}
