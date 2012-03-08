/**
 * @file  references.h
 * @brief System header declaring a structure for working with C referencers.
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

#ifndef _REFERENCES__H
#define _REFERENCES__H

#include <string>
using std::string;

namespace jdi {
  /**
    @struct jdi::ref_stack
    @brief  A stack-like structure representing C referencers.
    
    A reference stack is an info stack stored with any typed definition.
    The stack can depict any of C's standard referencers; the asterisk
    pointer symbol (*), the ampersand reference symbol (&), bracket array
    bound indicators ([]), and function parameter parentheses ((*)()).
    
    The stack does not depict parentheses used in grouping. Instead, the
    grouped referencers are simply added to the stack in the correct order.
    
    The stack is ordered such that each dereference symbol (listed above)
    can be evaluated simply by popping one item from a copy of the stack,
    performing each action as it is removed.
    
    As an example, this is the stack for int* (*(*a)[10][12])[15]:
    * RT_POINTERTO
    * RT_ARRAYBOUND(10)
    * RT_ARRAYBOUND(12)
    * RT_POINTERTO
    * RT_ARRAYBOUND(15)
    * RT_POINTERTO
  **/
  struct ref_stack {
    /// Types of referencers you'll find on this stack
    enum ref_type {
      RT_POINTERTO, ///< This referencer is a pointer-to asterisk, (*).
      RT_REFERENCE, ///< This referencer is a reference ampersand (&).
      RT_ARRAYBOUND, ///< This referencer is an array boundary subscript, [].
      RT_FUNCTION ///< This referencer is a set of function parameters.
    };
    
    /// Parameter storage container type. Guaranteed to have a push_back(full_type) method.
    struct parameter_ct;
    
    /// Iterator type, complying with C++11; iterates elements in the stack from top to bottom.
    class iterator;
    
    /// Node type.
    class node {
      node* previous; ///< The node beneath this node in the stack.
      friend struct ref_stack;
      friend struct ref_stack::iterator;
      public:
        ref_type type; ///< The type of this node.
        size_t arraysize(); ///< Return the size of this array if and only if type == RT_ARRAYBOUND. Undefined behavior otherwise.
        node(node* p, ref_type rt); ///< Allow constructing a new node easily.
        ~node(); ///< Non-virtual destructor which must call appropriate child constructor.
    };
    struct node_array;
    struct node_func;
    
    /// Push a node onto this stack by a given type.
    /// @param reference_type The type of this reference; should be either \c RT_REFERENCE or \c RT_POINTERTO.
    void push(ref_type reference_type);
    /// Push an array node onto this stack by a given type, with extra info.
    /// @param array_size  The number of elements in this array, or -1 for unspecified.
    void push_array(int array_size);
    /// Append a stack to the top of this stack, consuming it.
    void append(ref_stack &rf);
    /// Clear the stack, undoing all referencers.
    void clear();
    
    /// Wrapper to the copy() method so operator= doesn't leak and bite someone in the ass.
    ref_stack& operator= (const ref_stack& rf);
    /// Constructor wrapper to the copy() method.
    ref_stack(const ref_stack&);
    
    /// Return iterator from topmost item.
    iterator begin();
    /// Return invalid iterator for comparison.
    iterator end();
    
    ref_stack(); ///< Default contructor. Zeroes pointers.
    ~ref_stack(); ///< Default destructor. Frees the stack.
    
    
    struct iterator {
      private:
        node* n; ///< The node to which we are pointing.
        iterator(node*); ///< Utility constructor for use in begin().
        friend iterator ref_stack::begin(); ///< Let the begin() function use this constructor.
        friend iterator ref_stack::end(); ///< Let the end() function use this constructor.
      public:
        node* operator*(); ///< Get the current node pointer.
        node* operator->(); ///< Treat iterator as current node pointer.
        iterator operator++(int); ///< Increment iterator; move to next element on the stack. @return Return copy of iterator before call.
        iterator &operator++(); ///< Increment iterator; move to next element on the stack. @return Returns self.
        operator bool(); ///< Allow quick checking for invalidity; this iterator class does not use sentinel nodes.
    };
    
    string name; ///< The name of the object with the contained referencers.
    private:
      node *bottom; ///< The bottommost node on the list; used in the prepend method.
      node *top; ///< The topmost node on the list, for everything else.
      void copy(const ref_stack &rf); ///< Make a copy of the given ref_stack, DISCARDING any stored nodes! Call clear() first.
  };
}

//=========================================================================================================
//===: Specializations with extended dependencies :========================================================
//=========================================================================================================

#include <Storage/definition.h>

namespace jdi {  
  struct ref_stack::parameter_ct: public vector<jdi::full_type> {};
  struct ref_stack::node_array: ref_stack::node {
    size_t bound;
    static const size_t nbound = size_t(-1);
  };
  struct ref_stack::node_func: ref_stack::node {
    parameter_ct params;
    ~node_func();
  };
}

#endif
