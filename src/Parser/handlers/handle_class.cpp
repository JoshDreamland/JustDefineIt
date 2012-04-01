/**
 * @file handle_class.cpp
 * @brief Source implementing a function to handle class and structure declarations.
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

#include <cstdio>
#include <Parser/bodies.h>

static unsigned anon_count = 1111111;
int jdip::context_parser::handle_class(definition_scope *scope, token_t& token, int inherited_flags)
{
  unsigned protection = 0;
  unsigned incomplete = DEF_INCOMPLETE;
  bool already_complete = false;
  if (token.type == TT_CLASS)
     protection = DEF_PRIVATE;
  #ifdef DEBUG_MODE
  else if (token.type != TT_STRUCT)
    token.report_error(herr, "PARSE ERROR: handle_class invoked with non-class, non-struct token.");
  #endif
  
  token = read_next_token(scope);
  
  string classname;
  if (token.type == TT_IDENTIFIER)
    classname = string((const char*)token.extra.content.str, token.extra.content.len);
  else {
    char buf[32];
    sprintf(buf, "<anonymous%08d>", anon_count++);
    classname = buf;
  }
  
  definition_class *nclass;
  pair<definition_scope::defiter, bool> dins = scope->members.insert(pair<string,definition*>(classname,NULL));
  if (dins.second) // If a new definition key was created, then allocate a new class representation for it.
    dins.first->second = nclass = new definition_class(classname,scope, DEF_CLASS | DEF_TYPENAME | inherited_flags);
  else { // Make sure the class is incomplete
    nclass = (definition_class*)dins.first->second;
    if (not(dins.first->second->flags & DEF_CLASS)) {
      token.report_error(herr, "Attempting to redeclare `" + classname + "' as a class");
      return 1;
    }
    already_complete = nclass->flags & DEF_INCOMPLETE;
  }
  
  token = read_next_token(scope);
  if (token.type == TT_COLON) {
    if (already_complete) {
      token.report_error(herr, "Attempting to add ancestors to complete class `" + classname + "'");
      return 1;
    }
    incomplete = 0;
    do {
      unsigned iprotection = protection;
      token = read_next_token(scope);
      if (token.type == TT_PUBLIC)
        iprotection = 0,
        token = read_next_token(scope);
      else if (token.type == TT_PRIVATE)
        iprotection = DEF_PRIVATE,
        token = read_next_token(scope);
      else if (token.type == TT_PROTECTED)
        iprotection = DEF_PRIVATE,
        token = read_next_token(scope);
      if (token.type != TT_DECLARATOR or not(token.extra.def->flags & DEF_CLASS)) {
        string err = "Ancestor class name expected";
        if (token.type == TT_DECLARATOR) err += "; `" + token.extra.def->name + "' does not name a class";
        if (token.type == TT_IDENTIFIER) err += "; `" + string((const char*)token.extra.content.str,token.extra.content.len) + "' does not name a type";
        token.report_error(herr, err);
      }
      nclass->ancestors.push_back(definition_class::ancestor(iprotection, token.extra.def));
      token = read_next_token(scope);
    }
    while (token.type == TT_COMMA);
  }
  
  if (token.type == TT_LEFTBRACE)
  {
    incomplete = 0;
    if (handle_scope(nclass, token, protection)) return 1;
    if (token.type != TT_RIGHTBRACE) {
      token.report_error(herr, "Expected closing brace to class `" + classname + "'");
      return 1;
    }
  }
  else if (!incomplete) {
    token.report_error(herr, "Expected class body here after parents named");
    return 1;
  }
  
  nclass->flags |= incomplete;
  return 0;
}

