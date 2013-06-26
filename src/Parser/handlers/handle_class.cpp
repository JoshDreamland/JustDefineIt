/**
 * @file handle_class.cpp
 * @brief Source implementing a function to handle class and structure declarations.
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

#include <cstdio>
#include <Parser/bodies.h>
#include <API/compile_settings.h>


using namespace jdip;
#define alloc_class() new definition_class(classname,scope, DEF_CLASS | DEF_TYPENAME | inherited_flags)
static inline definition_class* insnew(definition_scope *const &scope, int inherited_flags, const string& classname, const token_t &token, error_handler* const& herr, context *ct) {
  definition_class* nclass = NULL;
  decpair dins = scope->declare(classname);
  if (!dins.inserted) {
    if (dins.def->flags & DEF_TYPENAME) { // This error is displayed because if the class existed earlier when we were checking, we'd have gotten a different token.
      token.report_error(herr, "Class `" + classname + "' instantiated inadvertently during parse by another thread. Freeing.");
      delete ~dins.def;
    }
    else {
      dins = ct->declare_c_struct(classname);
      if (dins.inserted)
        goto my_else;
      if (dins.def->flags & DEF_CLASS)
        nclass = (definition_class*)dins.def;
      else {
        #if FATAL_ERRORS
          return NULL;
        #else
          delete ~dins.def;
          goto my_else;
        #endif
      }
    }
  } else { my_else:
    dins.def = nclass = alloc_class();
  }
  return nclass;
}


int jdip::context_parser::handle_class_inheritance(definition_scope *scope, token_t& token, definition_class *recipient, unsigned default_protection) {
  do {
    unsigned iprotection = default_protection;
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
    if (token.type != TT_DECLARATOR and token.type != TT_DEFINITION) {
      string err = "Ancestor class name expected";
      if (token.type == TT_DECLARATOR) err += "; `" + token.def->name + "' does not name a class";
      if (token.type == TT_IDENTIFIER) err += "; `" + token.content.toString() + "' does not name a type";
      token.report_error(herr, err);
      return 1;
    }
    full_type ft = read_type(lex, token, scope, this, herr);
    if (!ft.def) {
      token.report_errorf(herr, "Expected class name to inherit before %s");
      return 1;
    }
    if (not(ft.def->flags & DEF_CLASS)) {
      if (ft.def->flags & DEF_TEMPPARAM) {
        definition_tempparam *tp = (definition_tempparam*)ft.def;
        tp->must_be_class = true;
      }
      else {
        token.report_errorf(herr, "Expected class name to inherit before %s");
        return 1;
      }
    }
    else {
      if (ft.flags or ft.refs.size())
        token.report_warning(herr, "Extra modifiers to inherited class ignored");
    }
    recipient->ancestors.push_back(definition_class::ancestor(iprotection, (definition_class*)ft.def));
  }
  while (token.type == TT_COMMA);
  
  return 0;
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
  
  definition *dulldef = NULL;
  if (token.type == TT_IDENTIFIER) {
    classname = token.content.toString();
    token = read_next_token(scope);
  }
  else if (token.type == TT_DEFINITION) {
    classname = token.content.toString();
    dulldef = token.def;
    token = read_next_token(scope);
  }
  else if (token.type == TT_DECLARATOR) {
    nclass = (jdi::definition_class*)token.def;
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
            nclass = NULL;
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
    classname = buf; // I love std::string. Even if I'm lazy for it.
  }
  
  if (dulldef)
  {
    if (dulldef->parent == scope)
    {
      if (dulldef->flags & DEF_TEMPLATE) {
        token.report_error(herr, "Cannot redeclare template `" + dulldef->name + "' as class in this scope; did you mean to specialize it?");
        return NULL;
      }
      else {
        token.report_error(herr, "Cannot redeclare `" + dulldef->name + "' as class in this scope");
        return NULL;
      }
    }
  }
  
  if (!nclass)
    if (not(nclass = insnew(scope,inherited_flags,classname,token,herr,this)))
      return NULL;
  
  // Handle inheritance
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
    
    if (handle_class_inheritance(scope, token, nclass, protection))
      return NULL;
  }
  
  // Handle implementation
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
      FATAL_RETURN(NULL);
    }
    nclass->flags &= ~DEF_INCOMPLETE;
    if (handle_scope(nclass, token, protection))
      FATAL_RETURN(NULL);
    if (token.type != TT_RIGHTBRACE) {
      token.report_error(herr, "Expected closing brace to class `" + classname + "'");
      FATAL_RETURN(NULL);
    }
    token = read_next_token(scope);
  }
  else // Sometimes, it isn't okay to not specify a structure body.
    if (!incomplete) { // The only way incomplete is zero in this instance is if it was set in : handler.
      token.report_errorf(herr, "Expected class body here (before %s) after parents named");
      FATAL_RETURN(NULL);
    }
  
  nclass->flags |= incomplete;
  return nclass;
}

