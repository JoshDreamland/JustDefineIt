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
#include <API/compile_settings.h>


using namespace jdip;
#define alloc_class() new definition_class(classname,scope, DEF_CLASS | DEF_TYPENAME | inherited_flags)
static inline definition_class* insnew(definition_scope *const &scope, int inherited_flags, const string& classname, const token_t &token, error_handler* const& herr, context *ct) {
  definition_class* nclass = NULL;
  pair<definition_scope::defiter, bool> dins = scope->members.insert(pair<string,definition*>(classname,NULL));
  if (!dins.second) {
    if (dins.first->second->flags & DEF_TYPENAME) {
      token.report_error(herr, "Class `" + classname + "' instantiated inadvertently during parse by another thread. Freeing.");
      delete dins.first->second;
    }
    else {
      dins = ct->c_structs.insert(pair<string,definition*>(classname,NULL));
      if (dins.second)
        goto my_else;
      if (dins.first->second->flags & DEF_CLASS)
        nclass = (definition_class*)dins.first->second;
      else {
        #if FATAL_ERRORS
          return NULL;
        #else
          delete dins.first->second;
          goto my_else;
        #endif
      }
    }
  } else { my_else:
    dins.first->second = nclass = alloc_class();
  }
  return nclass;
}

static unsigned anon_count = 1;
jdi::definition_class* jdip::context_parser::handle_class(definition_scope *scope, token_t& token, int inherited_flags)
{
  unsigned protection = 0;
  if (token.type == TT_CLASS)
     protection = DEF_PRIVATE;
  #ifdef DEBUG_MODE
  else if (token.type != TT_STRUCT)
    token.report_error(herr, "PARSE ERROR: handle_class invoked with non-class, non-struct token.");
  #endif
  
  token = read_next_token(scope);
  
  string classname;
  definition_class *nclass = NULL; // The class definition. Will be NULL until allocation, unless the class was already declared.
  bool already_complete = false; // True if nclass is non-NULL, and the class is already implemented.
  bool will_redeclare = false; // True if this class is from another scope; so true if implementing this class will allocate it.
  unsigned incomplete = DEF_INCOMPLETE; // DEF_INCOMPLETE if this class has a body, zero otherwise.
  
  // TRUTH TABLE
  // nclass    already_complete   will_redeclare  Meaning
  // NULL      *                  *               Nothing declared. Allocate new ASAP.
  // Non-NULL  False              False           Incomplete class in this scope; may be implemented.
  // Non-NULL  False              True            Incomplete class in another scope; can be implemented new in this scope.
  // Non-NULL  True               False           Complete class in this scope. MUST be used as a type, not implemented.
  // Non-NULL  True               True            Complete class in another scope; can be redeclared (reallocated and reimplemented) in this scope.
  
  if (token.type == TT_IDENTIFIER) {
    classname = string((const char*)token.extra.content.str, token.extra.content.len);
    token = read_next_token(scope);
  }
  else if (token.type == TT_DECLARATOR) {
    nclass = (jdi::definition_class*)token.extra.def;
    classname = nclass->name;
    if (not(nclass->flags & DEF_CLASS)) {
      if (nclass->parent == scope) {
        pair<definition_scope::defiter, bool> dins = c_structs.insert(pair<string,definition*>(classname, NULL));
        if (dins.second)
          dins.first->second = nclass = alloc_class();
        else {
          if (dins.first->second->flags & DEF_CLASS)
            nclass = (definition_class*)dins.first->second;
          else {
            token.report_error(herr, "Attempt to redeclare `" + classname + "' as class in this scope");
            FATAL_RETURN(NULL);
          }
        }
      }
      else
        nclass = NULL;
    }
    else {
      will_redeclare = nclass->parent != scope;
      already_complete = not(nclass->flags & DEF_INCOMPLETE);
    }
    token = read_next_token(scope);
  }
  else {
    char buf[32];
    sprintf(buf, "<anonymousClass%08d>", anon_count++);
    classname = buf;
  }
  
  if (!nclass)
    if (not(nclass = insnew(scope,inherited_flags,classname,token,herr,this)))
      return NULL;
  
  if (token.type == TT_COLON) {
    if (will_redeclare) {
      will_redeclare = false;
      if (not(nclass = insnew(scope,inherited_flags,classname,token,herr,this)))
        return NULL;
    }
    else if (already_complete) {
      token.report_error(herr, "Attempting to add ancestors to previously defined class `" + classname + "'");
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
      nclass->ancestors.push_back(definition_class::ancestor(iprotection, (definition_class*)token.extra.def));
      token = read_next_token(scope);
    }
    while (token.type == TT_COMMA);
  }
  
  if (token.type == TT_LEFTBRACE)
  {
    incomplete = 0;
    if (will_redeclare) {
      will_redeclare = false;
      if (not(nclass = insnew(scope,inherited_flags,classname,token,herr,this)))
        return NULL;
    }
    else if (already_complete) {
      token.report_error(herr, "Attempting to add members to previously defined class `" + classname + "'");
    }
    if (handle_scope(nclass, token, protection)) FATAL_RETURN(NULL);
    if (token.type != TT_RIGHTBRACE) {
      token.report_error(herr, "Expected closing brace to class `" + classname + "'");
      FATAL_RETURN(NULL);
    }
    token = read_next_token(scope);
  }
  else
    if (!incomplete) { // The only way incomplete is zero in this instance is if it was set in : handler.
      token.report_error(herr, "Expected class body here after parents named");
      FATAL_RETURN(NULL);
    }
  
  nclass->flags |= incomplete;
  return nclass;
}

