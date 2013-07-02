/**
 * @file  definition.cpp
 * @brief System source implementing arbitrary components of the definition storage class.
 * 
 * This file is likely used by absolutely everything in the parse system, as
 * it is the medium through which output definitions are created and manipulated.
 * 
 * @section License
 * 
 * Copyright (C) 2011-2013 Josh Ventura
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
#include <Parser/handlers/handle_function_impl.h>
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
  definition_typed::~definition_typed() {}
  
  definition_function::definition_function(string n, definition* p, definition* tp, ref_stack &rf, unsigned int typeflags, int flgs): 
    definition_typed(n, p, tp, rf, typeflags, flgs | DEF_FUNCTION), implementation(NULL) {
    arg_key k(referencers);
    overload(k, this, def_error_handler);
  }
  
  definition *definition_function::overload(arg_key &key, definition_function* ovrl, error_handler *herr) {
    pair<overload_iter, bool> ins = overloads.insert(pair<arg_key,definition_function*>(key, ovrl));
    if (!ins.second) {
      if (ins.first->second->implementation) {
        if (ovrl->implementation) {
          herr->error("Reimplementation of function; old implementation discarded");
          delete_function_implementation(ovrl->implementation);
          ovrl->implementation = ins.first->second->implementation;
        }
      }
      else
        ins.first->second->implementation = ovrl->implementation;
      delete ovrl;
    }
    return ins.first->second;
  }
  void definition_function::overload(definition_template* ovrl) {
    template_overloads.push_back(ovrl);
  }
  
  definition_function::~definition_function() {
    for (overload_iter it = overloads.begin(); it != overloads.end(); ++it)
      if (it->second != this) delete it->second;
    for (vector<definition_template*>::iterator it = template_overloads.begin(); it != template_overloads.end(); ++it)
      delete *it;
  }
  
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
  definition *definition_class::look_up(string sname) {
    defiter it = members.find(sname);
    if (it != members.end())
      return it->second;
    if ((it = using_general.find(sname)) != using_general.end())
      return it->second;
    definition *res;
    for (using_node* n = using_front; n; n = n->next)
      if ((res = n->use->find_local(sname)))
        return res;
    for (vector<ancestor>::iterator ait = ancestors.begin(); ait != ancestors.end(); ++ait)
      if ((res = ait->def->find_local(sname)))
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
  
  definition *definition_tempparam::look_up(string sname) {
    must_be_class = true;
    return definition_class::look_up(sname);
  }
  decpair definition_tempparam::declare(string sname, definition *def) {
    must_be_class = true;
    return definition_class::declare(sname, def);
  }
  definition *definition_tempparam::find_local(string sname) {
    must_be_class = true;
    pair<defmap::iterator, bool> insp = members.insert(defmap::value_type(sname, NULL));
    if (insp.second) {
      insp.first->second = new definition_tempparam(sname, this);  
    }
    return insp.first->second;
  }
  
  definition_scope::using_node *definition_scope::use_namespace(definition_scope *ns) {
    using_node *res;
    if (using_back)
      res = using_back = new using_node(ns, using_back);
    else
      res = using_front = using_back = new using_node(ns);
    return res;
  }
  void definition_scope::unuse_namespace(definition_scope::using_node *n) {
    if (using_back == n) using_back = n->prev;
    if (using_front == n) using_front = n->next;
    if (n->next) n->next->prev = n->prev;
    if (n->prev) n->prev->next = n->next;
    delete n;
  }
  
  void definition_scope::use_general(string n, definition *def) {
    using_general.insert(pair<string,definition*>(n,def));
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
  definition_scope::using_node::using_node(definition_scope* scope): use(scope), next(NULL), prev(NULL) { }
  definition_scope::using_node::using_node(definition_scope* scope, using_node* nprev): use(scope), next(nprev->next), prev(nprev) { nprev->next = this; }
  
  definition_class::ancestor::ancestor(unsigned protection_level, definition_class* inherit_from): protection(protection_level), def(inherit_from) {}
  definition_class::ancestor::ancestor() {}
  definition_class::definition_class(string classname, definition_scope* prnt, unsigned flgs): definition_scope(classname, prnt, flgs) {}
  
  definition_union::definition_union(string classname, definition_scope* prnt, unsigned flgs): definition_scope(classname, prnt, flgs) {}
  
  //definition_valued::definition_valued(string vname, definition *parnt, definition* tp, unsigned int flgs, value &val): definition_typed(vname, parnt, tp, 0, flgs | DEF_VALUED), value_of(val) {}
  definition_valued::definition_valued(string vname, definition *parnt, definition* tp, unsigned tflgs, unsigned int flgs, value &val): definition_typed(vname, parnt, tp, tflgs, flgs | DEF_VALUED), value_of(val) {}
  
  definition_enum::definition_enum(string classname, definition_scope* parnt, unsigned flgs): definition_class(classname, parnt, flgs | DEF_ENUM) {}
  
  definition_template::definition_template(string n, definition *p, unsigned f): definition_scope(n, p, f | DEF_TEMPLATE), def(NULL)  {}
  definition_template::~definition_template() {
    for (size_t i = 0; i < params.size(); ++i)
      delete params[i];
    for (speciter i = specializations.begin(); i != specializations.end(); ++i)
      delete i->second;
    for (depiter i = dependents.begin(); i != dependents.end(); ++i)
      delete *i;
    delete def;
  }
  
  definition_tempparam::definition_tempparam(string p_name, definition_scope* p_parent, unsigned p_flags): definition_class(p_name, p_parent, p_flags | DEF_TEMPPARAM) {}
  definition_tempparam::definition_tempparam(string p_name, definition_scope* p_parent, full_type &tp, unsigned p_flags): definition_class(p_name, p_parent, p_flags | DEF_TEMPPARAM | DEF_TYPENAME), default_type(tp) {}
  definition_tempparam::definition_tempparam(string p_name, definition_scope* p_parent, full_type &tp, AST* defval, unsigned p_flags): definition_class(p_name, p_parent, p_flags | DEF_TEMPPARAM), default_value(defval), default_type(tp) {}
  
  definition* definition_template::instantiate(arg_key& key) {
    pair<arg_key,definition*> insme(key,NULL);
    /*pair<map<arg_key,definition*>::iterator, bool> ins =*/ instantiations.insert(insme);
    return def;//ins.first->second;
  }
  
  definition_template *definition_template::specialize(arg_key& key, definition_template *spec) {
    pair<speciter, bool> ins = specializations.insert(specmap::value_type(key, spec));
    if (ins.second) return spec;
    return ins.first->second;
  }
  
  definition arg_key::abstract("<unspecified>", NULL, 0);
  
  bool arg_key::operator<(const arg_key& other) const {
    for (arg_key::node *i = values, *j = other.values; j != other.endv; ++i) {
      if (i == endv) return true;
      if (i->type == AKT_VALUE) {
        if (j->type != AKT_VALUE) return false;
        if (i->val() < j->val()) return true;
        if (j->val() < i->val()) return false;
      }
      else if (i->type == AKT_FULLTYPE) { // I is not a value; ie, it is a full_type
        if (j->type != AKT_FULLTYPE) return true; 
        if (i->ft() < j->ft()) return true;
        if (j->ft() < i->ft()) return false;
      }
    } return false;
  }
  
  void arg_key::mirror(definition_template *temp) {
    for (size_t i = 0; i < temp->params.size(); ++i)
      if (temp->params[i]->flags & DEF_TYPENAME) {
        new(&values[i].data) full_type(temp->params[i]->default_type);
        values[i].type = AKT_FULLTYPE;
      }
      else {
        if (temp->params[i]->default_value) {
          new(&values[i].data) value(temp->params[i]->default_value->eval());
          if ((*(value*)&values[i].data).type == VT_NONE)
            fprintf(stderr, "Expression in template parameter could not be evaluated.");
        }
        else new(&values[i].data) value();
        values[i].type = AKT_VALUE;
      }
  }
  
  void arg_key::put_final_type(size_t argnum, const full_type &type) { new (&values[argnum].data) full_type(); values[argnum].ft().copy(type); values[argnum].type = AKT_FULLTYPE; }
  void arg_key::swap_final_type(size_t argnum, full_type &type)      { new (&values[argnum].data) full_type(); values[argnum].ft().swap(type); values[argnum].type = AKT_FULLTYPE; }
  void arg_key::put_type(size_t argnum, const full_type &type) {
    if (type.def and type.def->flags & DEF_TYPED and ((definition_typed*)type.def)->type) {
      // Copy the type we were given
      full_type ft; ft.refs.copy(type.refs);
      // Copy the referencers that our type has of its own; not ref_stack::referencers, but ref_stack::def->referencers.
      ft.refs.prepend(((definition_typed*)type.def)->referencers);
      // Tack on the modifiers
      ft.flags |= ((definition_typed*)type.def)->modifiers;
      // Change out the type
      ft.def = ((definition_typed*)type.def)->type;
      return swap_type(argnum, ft);
    }
    return put_final_type(argnum, type);
  }
  void arg_key::swap_type(size_t argnum, full_type &type) {
    if (type.def and type.def->flags & DEF_TYPED and ((definition_typed*)type.def)->type) {
      // Copy the referencers that our type has of its own; not ref_stack::referencers, but ref_stack::def->referencers.
      type.refs.prepend(((definition_typed*)type.def)->referencers);
      // Tack on the modifiers
      type.flags |= ((definition_typed*)type.def)->modifiers;
      // Change out the type
      type.def = ((definition_typed*)type.def)->type;
      return swap_type(argnum, type);
    }
    return swap_final_type(argnum, type);
  }
  void arg_key::put_value(size_t argnum, const value &val) {
    new(&values[argnum].data) value(val);
    values[argnum].type = AKT_VALUE;
  }
  
  /// Default constructor; mark values NULL.
  arg_key::arg_key(): values(NULL), endv(NULL) {}
  /// Construct with a size, reserving sufficient memory.
  arg_key::arg_key(size_t n): values(new node[n]), endv(values+n) {} // Word to the wise: Do not switch the order of this initialization.
  /// Construct from a ref_stack.
  arg_key::arg_key(const ref_stack& rf) {
    #ifdef DEBUG_MODE
      if (rf.empty()) {
        cerr << "Critical error." << endl;
        return;
      }
    #endif
    const ref_stack::node &n = rf.top();
    #ifdef DEBUG_MODE
      if (n.type != ref_stack::RT_FUNCTION) {
        cerr << "Critical error." << endl;
        return;
      }
    #endif
    
    const ref_stack::parameter_ct &p = ((ref_stack::node_func*)&n)->params;
    values = new node[p.size()]; endv = values + p.size();
    for (size_t i = 0; i < p.size(); ++i)
      this->put_type(i, p[i]);
  }
  /// Construct a copy.
  arg_key::arg_key(const arg_key& other): values(new node[other.endv-other.values]) {
    node *i = values;
    for (node *j = other.values; j != other.endv; ++i, ++j)
      *i = *j;
    endv = i;
  }
  /// Destruct, freeing items.
  arg_key::~arg_key() { delete[] values; }
  
  arg_key::node &arg_key::node::operator=(const node& other) {
    type = other.type;
    if (type == AKT_FULLTYPE)
      new(&data) full_type(other.ft());
    else
      new(&data) value(other.val());
    return *this;
  }
  arg_key::node::~node() { if (type == AKT_FULLTYPE) ((full_type*)&data)->~full_type(); else if (type == AKT_VALUE) ((value*)&data)->~value(); }
  
  definition_atomic::definition_atomic(string n, definition* p, unsigned int f, size_t size): definition_scope(n,p,f), sz(size) {}
  
  definition_hypothetical::definition_hypothetical(string n, definition_scope *p, unsigned f, AST* d): definition_class(n,p,f|DEF_HYPOTHETICAL), def(d) {}
  definition_hypothetical::definition_hypothetical(string n, definition_scope *p, AST* d): definition_class(n,p,DEF_HYPOTHETICAL), def(d) {}
  definition_hypothetical::~definition_hypothetical() { delete def; }
  
  
  using_scope::using_scope(string n, definition_scope* u): definition_scope(n, u, DEF_NAMESPACE), using_me(u->use_namespace(this)) {}
  using_scope::~using_scope() { parent->unuse_namespace(using_me); }
  
  //========================================================================================================
  //======: Declare Functions :=============================================================================
  //========================================================================================================
  decpair::decpair(definition* *d, bool insd): def(d), inserted(insd) {}
  
  decpair definition_scope::declare(string n, definition* def) {
    inspair insp = members.insert(entry(n,def));
    return decpair(&insp.first->second, insp.second);
  }
  decpair definition_class::declare(string n, definition* def) {
    inspair insp = members.insert(entry(n,def));
    return decpair(&insp.first->second, insp.second);
  }
  
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
    return type? type->size_of() : 0;
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
  
  size_t definition_hypothetical::size_of() {
    cerr << "ERROR: sizeof() performed on dependent (hypothetical) type" << endl;
    return 0;
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
    if (!ancestors.empty()) {
      res += ": ";
      for (vector<ancestor>::iterator it = ancestors.begin(); it != ancestors.end(); ++it) {
        res += ((it->protection == DEF_PRIVATE)? "private " : (it->protection == DEF_PROTECTED)? "protected " : "public ");
        res += it->def->name + " ";
      }
    }
    if (levels and not(flags & DEF_INCOMPLETE))
      res += "\n", res += definition_scope::toString(dl(levels), indent);
    return res;
  }
  string definition_enum::toString(unsigned levels, unsigned indent) {
    const string inds(indent, ' ');
    string res = inds + "enum " + name + ": " + (type? type->name + " " : "");
    if (levels) {
      res += "{\n";
      string sinds(indent+2, ' ');
      bool first = true;
      for (vector<const_pair>::iterator it = constants.begin(); it != constants.end(); ++it) {
        if (!first) res += ",\n";
        res += sinds + it->def->name + " = " + it->def->value_of.toString();
        first = false;
      }
      res += "\n" + inds + "}";
    }
    else res += "{ ... }";  
    return res;
  }
  string definition_function::toString(unsigned levels, unsigned indent) {
    string res = definition_typed::toString(levels, indent);
    if (overloads.size() > 1)
      res += " (And " + tostr(overloads.size()-1) + " overloads)";
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
    for (piterator it = params.begin(); it != params.end(); ++it) {
      definition_tempparam *d = *it;
      if (!first) res += ", ";
      if (d->flags & DEF_TYPENAME) {
        res += d->name.empty()? "typename" : "typename " + d->name;
        if (d->default_type.def)
          res += " = " + d->default_type.toString();
      }
      else {
        res += (d->default_type.def? d->default_type.toString() : "<ERROR>");
        if (d->flags & DEF_VALUED)
          res += " = " + ((definition_valued*)d)->value_of;
      }
      first = false;
    }
    res += "> ";
    res += def? def->toString(levels, indent): "<null>";
    return res;
  }
  string definition_typed::toString(unsigned, unsigned indent) {
    string res(indent, ' ');
    if (flags & DEF_TYPENAME) res += "typedef ";
    res += type? typeflags_string(type, modifiers) : "<NULL>";
    res += " ";
    res += referencers.toStringLHS() + name + referencers.toStringRHS() + ";";
    return res;
  }
  string definition_union::toString(unsigned levels, unsigned indent) {
    string res = "union " + name + definition_scope::toString(levels,indent);
    return res;
  }
  string definition_valued::toString(unsigned, unsigned indent) {
    return string(indent, ' ') + referencers.toStringLHS() + name + referencers.toStringRHS() + " = " + value_of.toString();
  }
  string definition_hypothetical::toString(unsigned, unsigned indent) {
    return string(indent, ' ') + "template<typename " + parent->name + "> " + parent->name + "::" + name;
  }
}

#ifdef CUSTOM_MEMORY_MANAGEMENT
struct memblock {
  char data[sizeof(jdi::definition) << 7];
  char *atpt;
  memblock *next;
  static void *getnew(size_t sz);
  memblock(): atpt(data), next(NULL) {}
  ~memblock() { delete next; }
} first, *atmem = &first;

void *memblock::getnew(size_t sz) {
  char *np = atmem->atpt + sz;
  if (np < (char*)&atmem->atpt) {
    char *res = atmem->atpt;
    atmem->atpt = np;
    return res;
  }
  atmem = atmem->next = new memblock();
  return getnew(sz);
}

void *jdi::definition::operator new(size_t sz) {
  return atmem->getnew(sz);
}
void jdi::definition::operator delete(void*) {}
#endif
