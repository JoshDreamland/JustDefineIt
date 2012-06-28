/**
 * @file  definition.cpp
 * @brief System source implementing constructor.
 * 
 * This file is likely used by absolutely everything in the parse system, as
 * it is the medium through which output definitions are created and manipulated.
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

#include "definition.h"
#include <iostream>
#include <cstdio>
#include <System/builtins.h>
using namespace std;

namespace jdi {
  definition::definition(string n,definition* p,unsigned int f): flags(f), name(n), parent((definition_scope*)p) {}
  definition::definition(): flags(0), name(), parent(NULL) {}
  definition::~definition() {}
  
  ptrdiff_t definition::defcmp(definition *d1, definition *d2) {
    return d1 - d2;
  }
  
  definition_typed::definition_typed(string n, definition* p, definition* tp, unsigned int typeflags, int flgs): definition(n,p,flgs | DEF_TYPED), type(tp), referencers(), modifiers(typeflags) {}
  definition_typed::definition_typed(string n, definition* p, definition* tp, ref_stack &rf, unsigned int typeflags, int flgs): definition(n,p,flgs), type(tp), referencers(rf), modifiers(typeflags) {}
  
  definition_function::definition_function(string n, definition* p, definition* tp, ref_stack &rf, unsigned int typeflags, int flgs): 
    definition_typed(n, p, tp, rf, typeflags, flgs | DEF_FUNCTION) {}
  
  definition *definition_scope::look_up(string sname) {
    defiter it = members.find(sname);
    if (it != members.end())
      return it->second;
    if ((it = using_general.find(sname)) != using_general.end())
      return it->second;
    definition *res;
    for (using_node* n = using_front; n; n = n->next)
      if ((res = n->use->find_local(sname)))
        return res;
    if (parent == NULL)
      return NULL;
    return parent->look_up(sname);
  }
  definition *definition_scope::find_local(string sname) {
    defiter it = members.find(sname);
    if (it != members.end())
      return it->second;
    if ((it = using_general.find(sname)) != using_general.end())
      return it->second;
    definition *res;
    for (using_node* n = using_front; n; n = n->next)
      if ((res = n->use->find_local(sname)))
        return res;
    return NULL;
  }
  void definition_scope::use_namespace(definition_scope *ns) {
    if (using_back)
      using_back = new using_node(ns, using_back);
    else
      using_front = using_back = new using_node(ns);
  }
  void definition_scope::use_general(string n, definition *def) {
    using_general.insert(pair<string,definition*>(n,def));
  }
  void definition_scope::copy(const definition_scope* from) {
    remap_set n;
    for (defiter_c it = from->members.begin(); it != from->members.end(); it++) {
      pair<defiter,bool> dest = members.insert(pair<string,definition*>(it->first,NULL));
      if (dest.second) {
        dest.first->second = it->second->duplicate(n);
      }
    }
    remap(n);
  }
  definition_scope::definition_scope(): definition("",NULL,DEF_SCOPE), using_front(NULL), using_back(NULL) { }
  definition_scope::definition_scope(const definition_scope&): definition() {
    // TODO: Implement
  }
  definition_scope::definition_scope(string name_, definition *parent_, unsigned int flags_): definition(name_,parent_,flags_ | DEF_SCOPE), using_front(NULL), using_back(NULL) {}
  definition_scope::~definition_scope() {
    for (defiter it = members.begin(); it != members.end(); it++)
      delete it->second;
    for (using_node *n = using_front; n; ) {
      using_node *dm = n; n = n->next;
      delete dm;
    }
    members.clear();
  }
  definition_scope::using_node::using_node(definition_scope* scope): use(scope), next(NULL) { }
  definition_scope::using_node::using_node(definition_scope* scope, using_node* prev): use(scope), next(NULL) { prev->next = this; }
  
  definition_class::ancestor::ancestor(unsigned protection_level, definition_class* inherit_from): protection(protection_level), def(inherit_from) {}
  definition_class::ancestor::ancestor() {}
  definition_class::definition_class(string classname, definition_scope* prnt, unsigned flgs): definition_scope(classname, prnt, flgs) {}
  
  definition_union::definition_union(string classname, definition_scope* prnt, unsigned flgs): definition_scope(classname, prnt, flgs) {}
  
  //definition_valued::definition_valued(string vname, definition *parnt, definition* tp, unsigned int flgs, value &val): definition_typed(vname, parnt, tp, 0, flgs | DEF_VALUED), value_of(val) {}
  definition_valued::definition_valued(string vname, definition *parnt, definition* tp, unsigned tflgs, unsigned int flgs, value &val): definition_typed(vname, parnt, tp, tflgs, flgs | DEF_VALUED), value_of(val) {}
  
  definition_enum::definition_enum(string classname, definition_scope* parnt, unsigned flgs): definition_typed(classname, parnt, NULL, 0, flgs) {}
  
  definition_template::definition_template(string n, definition *p, unsigned f): definition(n, p, f | DEF_TEMPLATE), def(NULL)  {}
  definition_template::~definition_template() {
    for (size_t i = 0; i < params.size(); ++i)
      delete params[i];
    delete def;
  }
  definition* definition_template::instantiate(arg_key& key) {
    pair<arg_key&,definition*> insme(key,NULL);
    /*pair<map<arg_key,definition*>::iterator, bool> ins =*/ instantiations.insert(insme);
    return def;//ins.first->second;
  }
  bool definition_template::arg_key::operator<(const arg_key& other) const {
    for (definition **i = values, **j = other.values; *j; ++i) {
      if (!*i) return true;
      const ptrdiff_t comp = definition::defcmp(*i, *j);
      if (comp) return comp < 0;
    } return false;
  }
  void definition_template::arg_key::mirror(definition_template *temp) {
    for (size_t i = 0; i < temp->params.size(); ++i)
      if (temp->params[i]->flags & DEF_TYPENAME) {
        definition_typed* dt = (definition_typed*)temp->params[i];
        ref_stack dup; dup.copy(dt->referencers);
        values[i] = new definition_typed(dt->name, dt->parent, dt->type, dup, dt->modifiers, dt->flags);
      }
      else {
        definition_valued* dv = (definition_valued*)temp->params[i];
        values[i] = new definition_valued(dv->name, dv->parent, dv->type, dv->modifiers, dv->flags, dv->value_of);
      }
  }

  definition_atomic::definition_atomic(string n, definition* p, unsigned int f, size_t size): definition_scope(n,p,f), sz(size) {}

  //========================================================================================================
  //======: Re-map Functions :==============================================================================
  //========================================================================================================
  
  #ifdef DEBUG_MODE
    #define DEBUG_ONLY(x) x
  #else
    #define DEBUG_ONLY(x)
  #endif
  
  void definition::remap(const remap_set &) {}
  
  void definition_class::remap(const remap_set &n) {
    for (vector<ancestor>::iterator it = ancestors.begin(); it != ancestors.end(); ++it) {
      ancestor& an = *it;
      remap_set::const_iterator ex = n.find(an.def);
      if (ex != n.end()) {
        #ifdef DEBUG_MODE
          if (not(ex->second->flags & DEF_CLASS))
            cerr << "ERROR! Replacing `" << an.def->name << "' at " << ((void*)an.def)
                 << " with non-class `" << ex->second->name << "' at " << ((void*)ex->second) << endl;
        #endif
        an.def = (definition_class*)ex->second;
      }
    }
    definition_scope::remap(n);
  }
  
  void definition_enum::remap(const remap_set& DEBUG_ONLY(n)) {
    #ifdef DEBUG_MODE
    if (n.find(type) != n.end()) {
      cerr << "Why are you replacing `" << type->name << "'?" << endl;
    }
    #endif
  }
  
  void definition_function::remap(const remap_set& ) {
    
  }
  
  void definition_scope::remap(const remap_set &n) {
    for (defiter it = members.begin(); it != members.end(); ++it) {
      remap_set::const_iterator ex = n.find(it->second);
      if (ex == n.end())
        it->second->remap(n);
      else
        it->second = ex->second;
    }
  }
  
  void definition_template::remap(const remap_set &) {
    
  }
  
  void definition_typed::remap(const remap_set &) {
    
  }
  
  void definition_union::remap(const remap_set &) {
    
  }
  
  void definition_atomic::remap(const remap_set &) {}
  
  //========================================================================================================
  //======: Sizeof functions :==============================================================================
  //========================================================================================================
  
  size_t definition::size_of() { return 0; }

  size_t definition_class::size_of() {
    size_t sz = 0;
    for (defiter it = members.begin(); it != members.end(); ++it)
      if (not(it->second->flags & DEF_TYPENAME)) {
        size_t as = it->second->size_of();
        sz += as - 1;
        sz /= as; sz *= as;
        sz += as;
      }
    return sz;
  }

  size_t definition_enum::size_of() {
    return type->size_of();
  }

  size_t definition_function::size_of() {
    return 0;
  }

  size_t definition_scope::size_of() {
    size_t sz = 0;
    for (defiter it = members.begin(); it != members.end(); ++it)
      if (not(it->second->flags & DEF_TYPENAME))
        sz += it->second->size_of();
    return sz;
  }

  size_t definition_template::size_of() {
    cerr << "Attempt to take size of template `" << name << "'" << endl;
    return 0;
  }

  size_t definition_typed::size_of() {
    return type->size_of();
  }

  size_t definition_union::size_of() {
    size_t sz = 0;
    for (defiter it = members.begin(); it != members.end(); ++it)
      if (not(it->second->flags & DEF_TYPENAME))
        sz = max(sz, it->second->size_of());
    return sz;
  }
  
  size_t definition_atomic::size_of() {
    return sz;
  }
  
  //========================================================================================================
  //======: Duplicators :===================================================================================
  //========================================================================================================
  
  definition *definition::duplicate(remap_set &n) {
    definition* res = new definition(name, parent, flags);
    n[this] = res;
    return res;
  }
  
  definition *definition_class::duplicate(remap_set &n) {
    definition_class* res= new definition_class(name, parent, flags);
    res->definition_scope::copy(this);
    res->ancestors = ancestors;
    n[this] = res;
    return res;
  }
  
  definition *definition_enum::duplicate(remap_set &n) {
    definition_enum* res = new definition_enum(name, parent, flags);
    res->type = type;
    res->constants = constants;
    n[this] = res;
    return res;
  }
  
  function_overload* function_overload::duplicate() {
    function_overload *res = new function_overload();
    res->type.def = type.def;
    res->type.refs.copy(type.refs);
    res->type.flags = type.flags;
    res->declaration = declaration;
    return res;
  }
  
  definition *definition_function::duplicate(remap_set &n) {
    ref_stack dup; dup.copy(referencers);
    definition_function* res = new definition_function(name, parent, type, dup, modifiers, flags);
    res->overloads.reserve(overloads.size());
    for (vector<function_overload*>::iterator it = overloads.begin(); it != overloads.end(); ++it)
      res->overloads.push_back((*it)->duplicate());
    n[this] = res;
    return res;
  }
  
  definition* definition_scope::duplicate(remap_set &n) {
    definition_scope* res = new definition_scope(name, parent, flags);
    res->copy(this);
    n[this] = res;
    return res;
  }
  
  definition* definition_template::duplicate(remap_set &n) {
    definition_template* res = new definition_template(name, parent, flags);
    res->def = def->duplicate(n);
    res->specializations = specializations;
    res->instantiations = instantiations;
    res->params.reserve(params.size());
    for (vector<definition*>::iterator it = params.begin(); it != params.end(); ++it)
      res->params.push_back((*it)->duplicate(n));
    for (speciter it = res->specializations.begin(); it != res->specializations.end(); ++it) {
      definition *nd = it->second->duplicate(n);
      n[it->second] = nd; it->second = (definition_template*)nd;
    }
    for (institer it = res->instantiations.begin(); it != res->instantiations.end(); ++it) {
      definition *nd = it->second->duplicate(n);
      n[it->second] = nd; it->second = nd;
    }
    n[this] = res;
    return res;
  }
  
  definition* definition_typed::duplicate(remap_set &n) {
    definition_typed* res = new definition_typed(name, parent, type, modifiers, flags);
    res->referencers.copy(referencers);
    n[this] = res;
    return res;
  }
  
  definition* definition_union::duplicate(remap_set &n) {
    definition_union* res = new definition_union(name, parent, flags);
    res->definition_scope::copy(this);
    n[this] = res;
    return res;
  }
  
  definition* definition_atomic::duplicate(remap_set &) {
    return this;
  }
  
  //========================================================================================================
  //======: String printers :===============================================================================
  //========================================================================================================
  
  inline string tostr(int x) { char buf[16]; sprintf(buf, "%d", x); return buf; }
  inline unsigned dl(unsigned l) { return l == unsigned(-1)? l:l-1; }
  
  string definition::toString(unsigned, unsigned indent) {
    return string(indent, ' ') + "void " + name + ";";
  }
  string definition_atomic::toString(unsigned, unsigned indent) {
    return string(indent, ' ') + "typedef __atom__[" + tostr(sz) + "] " + name + ";";
  }
  string definition_class::toString(unsigned levels, unsigned indent) {
    const string inds(indent, ' ');
    string res = inds + "class " + name;
    if (!ancestors.empty())
      for (vector<ancestor>::iterator it = ancestors.begin(); it != ancestors.end(); ++it) {
        res += ((it->protection == DEF_PRIVATE)? "private " : (it->protection == DEF_PROTECTED)? "protected " : "public ");
        res += it->def->name + " ";
      }
    if (flags & DEF_INCOMPLETE)
      res += ";";
    else
      res += "\n", res += definition_scope::toString(dl(levels), indent);
    return res;
  }
  string definition_enum::toString(unsigned levels, unsigned indent) {
    const string inds(indent, ' ');
    string res = inds + "enum " + name + ": " + type->name;
    if (levels) {
      res += "{\n";
      string sinds(indent+2, ' ');
      bool first = true;
      for (map<string,definition*>::iterator it = constants.begin(); it != constants.end(); ++it) {
        if (!first) res += ",\n";
        res += sinds + it->first + " = " + ((definition_valued*)it->second)->value_of.toString();
        first = false;
      }
      res += "\n" + inds + "}";
    }
    else res += "{ ... }";  
    return res;
  }
  string definition_function::toString(unsigned levels, unsigned indent) {
    string res = definition_typed::toString(levels, indent);
    if (overloads.size())
      res += " (And " + tostr(overloads.size()) + " overloads)";
    return res;
  }
  string definition_scope::toString(unsigned levels, unsigned indent) {
    string inds(indent, ' '), res = inds;
    if (flags & DEF_NAMESPACE)
      res = name.empty()? "namespace " : "namespace " + name + " ";
    if (levels) {
      res += "{\n";
      for (defiter it = members.begin(); it != members.end(); ++it)
        res += it->second->toString(levels-1, indent+2) + "\n";
      res += inds + "}";
    }
    else res += "{ ... }";
    return res;
  }
  string definition_template::toString(unsigned levels, unsigned indent) {
    string res(indent, ' ');
    res += "template<";
    bool first = true;
    for (vector<definition*>::iterator it = params.begin(); it != params.end(); ++it) {
      definition_typed *d = (definition_typed*)*it;
      if (!first) res += ", ";
      if (d->flags & DEF_TYPENAME) {
        res += d->name.empty()? "typename" : "typename " + d->name;
        if (d->type)
          res += " = " + d->type->name;
      }
      else {
        res += d->type->name + d->name;
        if (d->flags & DEF_VALUED)
          res += " = " + ((definition_valued*)d)->value_of;
      }
      first = false;
    }
    res += "> ";
    res += def->toString(levels, indent);
    return res;
  }
  string definition_typed::toString(unsigned, unsigned indent) {
    return string(indent, ' ') + typeflags_string(type, modifiers) + " " + referencers.toStringLHS() + name + referencers.toStringRHS();
  }
  string definition_union::toString(unsigned levels, unsigned indent) {
    string res = "union " + name + definition_scope::toString(levels,indent);
    return res;
  }
  string definition_valued::toString(unsigned, unsigned indent) {
    return string(indent, ' ') + referencers.toStringLHS() + name + referencers.toStringRHS() + " = " + value_of.toString();
  }
}
