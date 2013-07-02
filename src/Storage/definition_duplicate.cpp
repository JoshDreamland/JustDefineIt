/**
 * @file  definition_duplicate.cpp
 * @brief System source implementing duplicate and remap components of the definition storage class.
 * 
 * This file is likely used by absolutely everything in the parse system, as
 * it is the medium through which output definitions are created and manipulated.
 * 
 * @section License
 * 
 * Copyright (C) 2011-2013 Josh Ventura
 * This file is part of JustDefineIt.
 * void definition_scope::copy(const definition_scope* from) {
    remap_set n;
    for (defiter_c it = from->members.begin(); it != from->members.end(); it++) {
      inspair dest = members.insert(entry(it->first,NULL));
      if (dest.second) {
        dest.first->second = it->second->duplicate(n);
      }
    }
    remap(n);
  }
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

namespace jdi {
  
  void definition_scope::copy(const definition_scope* from) {
    remap_set n;
    for (defiter_c it = from->members.begin(); it != from->members.end(); it++) {
      inspair dest = members.insert(entry(it->first,NULL));
      if (dest.second) {
        dest.first->second = it->second->duplicate(n);
      }
    }
    remap(n);
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
    n[this] = res;
    
    res->overloads = overloads;
    return res;
  }
  
  definition* definition_scope::duplicate(remap_set &n) {
    definition_scope* res = new definition_scope(name, parent, flags);
    n[this] = res;
    
    res->copy(this);
    return res;
  }
  
  definition* definition_template::duplicate(remap_set &n) {
    definition_template* res = new definition_template(name, parent, flags);
    n[this] = res;
    
    res->def = def->duplicate(n);
    res->specializations = specializations;
    res->instantiations = instantiations;
    res->params.reserve(params.size());
    for (piterator it = params.begin(); it != params.end(); ++it)
      res->params.push_back((definition_tempparam*)(*it)->duplicate(n));
    for (speciter it = res->specializations.begin(); it != res->specializations.end(); ++it) {
      definition *nd = it->second->duplicate(n);
      n[it->second] = nd; it->second = (definition_template*)nd;
    }
    for (institer it = res->instantiations.begin(); it != res->instantiations.end(); ++it) {
      definition *nd = it->second->duplicate(n);
      n[it->second] = nd; it->second = nd;
    }
    return res;
  }
  
  definition* definition_tempparam::duplicate(remap_set &n) {
    definition_tempparam* res = new definition_tempparam(name, parent, flags);
    n[this] = res;
    
    res->default_type = default_type;
    res->default_value = new AST(/*default_value*/); // FIXME: This drops information! Need an AST::duplicate.
    res->definition_class::copy(this);
    return res;
  }
  
  definition* definition_typed::duplicate(remap_set &n) {
    definition_typed* res = new definition_typed(name, parent, type, modifiers, flags);
    n[this] = res;
    
    res->referencers.copy(referencers);
    return res;
  }
  
  definition* definition_union::duplicate(remap_set &n) {
    definition_union* res = new definition_union(name, parent, flags);
    n[this] = res;
    
    res->definition_scope::copy(this);
    return res;
  }
  
  definition* definition_atomic::duplicate(remap_set &) {
    return this;
  }
  
  definition* definition_hypothetical::duplicate(remap_set &n) {
    definition_hypothetical* res = new definition_hypothetical(name, parent, flags, new AST(*def));
    n[this] = res; return res;
  }
  
  
  //========================================================================================================
  //======: Re-map Functions :==============================================================================
  //========================================================================================================
  
  #ifdef DEBUG_MODE
    #define DEBUG_ONLY(x) x
  #else
    #define DEBUG_ONLY(x)
  #endif
  
  template<class dc> inline dc *filter(dc *x, const remap_set &remap) {
    remap_citer it = remap.find(x);
    return it == remap.end()? x : (dc*)it->second;
  }
  
  void definition::remap(const remap_set &n) {
    parent = filter(parent, n);
  }
  
  void definition_scope::remap(const remap_set &n) {
    definition::remap(n);
    for (defiter it = members.begin(); it != members.end(); ++it) {
      remap_set::const_iterator ex = n.find(it->second);
      if (ex == n.end())
        it->second->remap(n);
      else
        it->second = ex->second;
    }
    for (using_node *un = using_front; un; un = un->next) {
      remap_set::const_iterator ex = n.find(un->use);
      if (ex == n.end())
        un->use = (definition_scope*)ex->second;
    }
    for (defiter it = using_general.begin(); it != using_general.end(); ++it) {
      remap_set::const_iterator ex = n.find(it->second);
      if (ex != n.end())
        it->second = ex->second;
    }
  }
  
  void definition_class::remap(const remap_set &n) {
    definition_scope::remap(n);
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
  }
  
  void definition_enum::remap(const remap_set& n) {
    #ifdef DEBUG_MODE
    if (n.find(type) != n.end()) {
      cerr << "Why are you replacing `" << type->name << "'?" << endl;
    }
    #endif
    type = filter(type, n);
    
    for (vector<const_pair>::iterator it = constants.begin(); it != constants.end(); ++it) {
      definition_valued *d = filter(it->def, n);
      if (it->def == d)
        it->def->remap(n);
      else {
        delete it->def;
        it->def = d;
      }
    }
  }
  
  void definition_function::remap(const remap_set& n) {
    definition_typed::remap(n);
    for (overload_iter it = overloads.begin(); it != overloads.end(); ++it)
      if (it->second != this)
        it->second->remap(n);
  }
  
  void definition_template::remap(const remap_set &n) {
    if (def)
      def->remap(n);
  }
  
  void definition_tempparam::remap(const remap_set &n) {
    // TODO: Implement
    (void)n;
  }
  
  void definition_typed::remap(const remap_set &n) {
    remap_set::const_iterator ex = n.find(type);
    if (ex != n.end())
      type = ex->second;
  }
  
  void definition_union::remap(const remap_set &) {
    
  }
  
  void definition_atomic::remap(const remap_set &) {}
  
  void definition_hypothetical::remap(const remap_set &) { cerr << "ERROR: Remap called on hypothetical type" << endl; }
  
  
  //========================================================================================================
  //======: AST Node Re-map Functions :=====================================================================
  //========================================================================================================
  
  void AST::AST_Node            ::remap(const remap_set&) {  }
  void AST::AST_Node_Scope      ::remap(const remap_set& n) { AST_Node_Binary::remap(n); }
  void AST::AST_Node_Unary      ::remap(const remap_set& n) { operand->remap(n); }
  void AST::AST_Node_sizeof     ::remap(const remap_set& n) { operand->remap(n); }
  void AST::AST_Node_Type       ::remap(const remap_set& n) { dec_type.def  = filter(dec_type.def,  n); }
  void AST::AST_Node_Cast       ::remap(const remap_set& n) { cast_type.def = filter(cast_type.def, n); }
  void AST::AST_Node_Binary     ::remap(const remap_set& n) { left->remap(n); right->remap(n); }
  void AST::AST_Node_Ternary    ::remap(const remap_set& n) { left->remap(n); right->remap(n); exp->remap(n); }
  void AST::AST_Node_new        ::remap(const remap_set& n) { type.def = filter(type.def, n); position->remap(n); bound->remap(n); }
  void AST::AST_Node_delete     ::remap(const remap_set& n) { AST_Node_Unary::remap(n); }
  void AST::AST_Node_Subscript  ::remap(const remap_set& n) { left->remap(n); index->remap(n); }
  
  void AST::AST_Node_Parameters ::remap(const remap_set& n) {
    func->remap(n);
    for (vector<AST_Node*>::iterator p = params.begin(); p != params.end(); ++p)
      (*p)->remap(n);
  }
  void AST::AST_Node_Array      ::remap(const remap_set& n) {
    for (vector<AST_Node*>::iterator e = elements.begin(); e != elements.end(); ++e)
      (*e)->remap(n);
  }
  
  void AST::AST_Node_Definition ::remap(const remap_set& n) {
    definition *d = filter(def, n);
    if (d != def)
      def = d;
    else def->remap(n);
  }
}
