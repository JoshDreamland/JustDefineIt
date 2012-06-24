/**
 * @file  references.cpp
 * @brief Source implementing methods for dealing with C referencers as a structure.
 * 
 * The asterisk pointer symbol (*), ampersand reference symbol (&), bracket
 * array bound indicators ([]), and function parameter parentheses ((*)())
 * are each unique types of references dealt with in this file.
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

#include "references.h"

#ifndef dbg_assert
  #ifdef DEBUG_MODE
    #include <assert.h>
    #define dbg_assert assert
  #else
    #define dbg_assert(x)
  #endif
#endif
#include <iostream>

namespace jdi {
  ref_stack::ref_stack(): ntop(NULL), nbottom(NULL), sz(0) {}
  ref_stack::ref_stack(ref_stack& rf): ntop(NULL), nbottom(NULL), sz(0) { swap(rf); }
  ref_stack::ref_stack(const ref_stack& rf) { cout << "IMPLICITLY DUPLICATED REF STACK" << endl; copy(rf); }
  ref_stack::~ref_stack() { clear(); }
  
  ref_stack &ref_stack::operator= (const ref_stack& rf) { clear(); cout << "IMPLICITLY DUPLICATED REF STACK" << endl; copy(rf); return *this; }
  
  ref_stack &ref_stack::operator= (ref_stack& rf) { swap(rf); return *this; }
  
  ref_stack::node::node(node* p, ref_type rt): previous(p), type(rt) {}
  ref_stack::node_array::node_array(node* p, size_t b): node(p,RT_ARRAYBOUND), bound(b) {}
  ref_stack::node_func::node_func(node* p, parameter_ct &ps): node(p,RT_FUNCTION), params() { params.swap(ps); }
  
  ref_stack::node::~node() { }
  ref_stack::node_func::~node_func() {}
  
  ref_stack::node* ref_stack::node::duplicate() {
    if (type == RT_ARRAYBOUND) return new node_array(NULL,((node_array*)this)->bound);
    if (type == RT_FUNCTION) return new node_func(NULL,((node_func*)this)->params);
    cout << "DUPLICATE CALLED" << endl;
    return new node(NULL,type);
  }
  
  size_t ref_stack::node::arraysize() {
    dbg_assert(this->type == RT_ARRAYBOUND);
    return ((node_array*)this)->bound;
  }
  
  ref_stack::node* ref_stack::iterator::operator*() { return n; }
  ref_stack::node* ref_stack::iterator::operator->() { return n; }
  ref_stack::iterator ref_stack::iterator::operator++(int) { iterator res = *this; n = n->previous; return res; }
  ref_stack::iterator &ref_stack::iterator::operator++() { n = n->previous; return *this; }
  ref_stack::iterator::operator bool() { return n; }
  ref_stack::iterator::iterator(ref_stack::node *nconstruct): n(nconstruct) { }
  
  ref_stack::iterator ref_stack::begin() { return ref_stack::iterator(ntop); }
  ref_stack::iterator ref_stack::end() { return ref_stack::iterator(NULL); }
  
  void ref_stack::push(ref_stack::ref_type reference_type) {
    ntop = new node(ntop, reference_type);
    if (!nbottom) nbottom = ntop;
    ++sz;
  }
  void ref_stack::push_array(size_t array_size) {
    node* bo = nbottom;
    nbottom = new node_array(NULL, array_size);
    if (bo) bo->previous = nbottom;
    else ntop = nbottom;
    ++sz;
  }
  void ref_stack::push_func(parameter_ct &parameters) {
    node* bo = nbottom;
    nbottom = new node_func(NULL, parameters);
    if (bo) bo->previous = nbottom;
    else ntop = nbottom;
    ++sz;
  }
  
  void ref_stack::copy(const ref_stack& rf) {
    name = rf.name;
    if (!rf.nbottom) {
      ntop = nbottom = NULL;
      return;
    }
    nbottom = ntop = rf.ntop->duplicate();
    for (node *c = rf.ntop->previous; c; c = c->previous) {
      nbottom->previous = c->duplicate();
      nbottom = nbottom->previous;
    }
    sz = rf.sz;
  }
  void ref_stack::swap(ref_stack& rf) {
    name.swap(rf.name);
    node *ts = ntop, *bs= nbottom;
    ntop = rf.ntop, nbottom = rf.nbottom;
    rf.ntop = ts, rf.nbottom = bs;
    const size_t ss = rf.sz;
    rf.sz = sz; sz = ss;
  }
  
  void ref_stack::append(ref_stack &rf) {
    if (!rf.nbottom) return; // Appending an empty stack is meaningless
    if (!nbottom) nbottom = rf.nbottom; // If we didn't have anything on our stack, our nbottom is now its nbottom.
    rf.nbottom->previous = ntop; // If we had anything on our stack, then our ntop item comes before its nbottom item.
    ntop = rf.ntop; // Since we threw that stack on ntop of ours, its ntop is now our ntop.
    rf.ntop = rf.nbottom = NULL; // Make sure it doesn't free what we just stole
    sz += rf.sz; rf.sz = 0; // Steal its size, too.
  }
  void ref_stack::append_nest(ref_stack &rf) {
    if (!rf.nbottom) {
      if (!rf.name.empty()) name = rf.name; // Grab the name, if it's meaningful
      return; // Appending an empty stack is meaningless
    }
    if (!nbottom) nbottom = rf.nbottom; // If we didn't have anything on our stack, our nbottom is now its nbottom.
    rf.nbottom->previous = ntop; // If we had anything on our stack, then our ntop item comes before its nbottom item.
    ntop = rf.ntop; // Since we threw that stack on ntop of ours, its ntop is now our ntop.
    rf.ntop = rf.nbottom = NULL; // Make sure it doesn't free what we just stole
    sz += rf.sz; rf.sz = 0; // Steal its size, too.
    name = rf.name; // Steal the name from the nested expression.
  }
  
  void ref_stack::clear() {
    for (node* n = ntop, *p; n; n = p) {
      p = n->previous;
      switch (n->type) {
        case RT_FUNCTION: delete (node_func*)n; break;
        case RT_ARRAYBOUND: delete (node_array*)n; break;
        case RT_POINTERTO: case RT_REFERENCE: default: delete n; break;
      }
    }
    sz = 0;
  }
  
  bool ref_stack::empty() { return !nbottom; }
  size_t ref_stack::size() { return sz; }
  
  ref_stack::node& ref_stack::top() { return *ntop; }
  ref_stack::node& ref_stack::bottom() { return *nbottom; }
  
  void ref_stack::parameter_ct::throw_on(parameter &ft) {
    enswap(ft);
  }
  
  void ref_stack::parameter::swap(ref_stack::parameter &param) {
    full_type::swap(param);
    
    register bool swb;
    swb = param.variadic;
    param.variadic = variadic;
    variadic = swb;
    
    swb = param.defaulted;
    param.defaulted = defaulted;
    defaulted = swb;
    
    value v = param.default_value;
    param.default_value = default_value;
    default_value = v;
  }
  
  void ref_stack::parameter::swap_in(full_type &param) {
    full_type::swap(param);
  }
}
