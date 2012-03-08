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

#include <assert.h>
#include "references.h"

namespace jdi {
  ref_stack::ref_stack(): bottom(NULL), top(NULL) {}
  ref_stack::ref_stack(const ref_stack& rf) { copy(rf); }
  ref_stack::~ref_stack() { clear(); }
  
  ref_stack &ref_stack::operator= (const ref_stack& rf) { copy(rf); return *this; }
  
  ref_stack::node::node(node* p, ref_type rt): previous(p), type(rt) {}
  
  ref_stack::node::~node() { if (type == RT_FUNCTION) ((node_func*)this)->~node_func(); }
  ref_stack::node_func::~node_func() {  }
  
  size_t ref_stack::node::arraysize() {
    #ifdef DEBUG_MODE
      assert(this->type == RT_ARRAYBOUND);
    #endif
    return ((node_array*)this)->bound;
  }
  
  ref_stack::node* ref_stack::iterator::operator*() { return n; }
  ref_stack::node* ref_stack::iterator::operator->() { return n; }
  ref_stack::iterator ref_stack::iterator::operator++(int) { iterator res = *this; n = n->previous; return res; }
  ref_stack::iterator &ref_stack::iterator::operator++() { n = n->previous; return *this; }
  ref_stack::iterator::operator bool() { return n; }
  ref_stack::iterator::iterator(ref_stack::node *nconstruct): n(nconstruct) { }
  
  ref_stack::iterator ref_stack::begin() { return ref_stack::iterator(top); }
  ref_stack::iterator ref_stack::end() { return ref_stack::iterator(NULL); }
  
  void ref_stack::push(ref_stack::ref_type reference_type) {
    top = new node(top, reference_type);
    if (!bottom) bottom = top;
  }
  
  void ref_stack::copy(const ref_stack& rf) {
    name = rf.name;
    if (!rf.bottom) {
      top = bottom = NULL;
      return;
    }
    bottom = top = new node(*rf.top);
    for (node *c = rf.top->previous; c; c = c->previous) {
      bottom->previous = new node(*c);
      bottom = bottom->previous;
    }
  }
  
  void ref_stack::append(ref_stack &rf) {
    if (!rf.bottom) return; // Appending an empty stack is meaningless
    if (!bottom) bottom = rf.bottom; // If we didn't have anything on our stack, our bottom is now its bottom.
    rf.bottom->previous = top; // If we had anything on our stack, then our top item comes before its bottom item.
    top = rf.top; // Since we threw that stack on top of ours, its top is now our top.
    rf.top = rf.bottom = NULL; // Make sure it doesn't free what we just stole
  }
  
  void ref_stack::clear() {
    for (node* n = top, *p; n; n = p) {
      p = n->previous;
      delete n;
    }
  }
}
