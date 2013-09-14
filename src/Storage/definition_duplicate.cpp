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
using std::cerr;
using std::cout;
using std::endl;

namespace jdi {
  struct dec_order_generic: definition_scope::dec_order_g {
    definition *mydef;
    dec_order_generic(definition *x): mydef(x) {}
    definition *def() { return mydef; }
  };
  
  void definition_scope::copy(const definition_scope* from, remap_set &n) {
    for (defiter_c it = from->members.begin(); it != from->members.end(); it++)
    #ifdef DEBUG_MODE
    if (it->second == NULL) {
      cerr << "COMPLETE FAILURE. `" << it->first << "' in `" << from->name << "' is NULL!" << endl;
      abort();
    } else
    #endif
    if (!(it->second->flags & (DEF_CLASS | DEF_ENUM | DEF_UNION))) {
      inspair dest = members.insert(entry(it->first,NULL));
      if (dest.second) {
        dest.first->second = it->second->duplicate(n);
      }
    }
    for (defiter_c it = from->c_structs.begin(); it != from->c_structs.end(); ++it) {
      inspair dest = c_structs.insert(entry(it->first, NULL));
      if (dest.second) {
        definition *cs = it->second->duplicate(n);
        dest.first->second = cs;
        declare(it->first, cs);
      }
    }
    dec_order = from->dec_order;
    for (orditer it = dec_order.begin(); it != dec_order.end(); ++it) {
        definition *def = (*it)->def();
      remap_iter rit = n.find(def);
      #ifdef DEBUG_MODE
        if (rit == n.end() && !(def->flags & DEF_HYPOTHETICAL))
          cerr << "FAILURE! Definition `" << def->name << "' in the order deque was not found in the remap set!" << endl;
      #endif
      *it = new dec_order_generic((rit == n.end())? def : rit->second);
    }
    using_general = from->using_general;
    if (from->using_front) {
      if (!using_back)
        using_front = using_back = new using_node(from->using_front->use, NULL);
      for (using_node *un = from->using_front; un; un = un->next) {
        bool news = true;
        for (using_node *nf = using_front; nf; nf = nf->next)
          if (nf->use == un->use) {
            news = false;
            break;
          }
        if (news)
          using_back = new using_node(un->use, using_back);
      }
    }
  }

  //========================================================================================================
  //======: Duplicators :===================================================================================
  //========================================================================================================
  
  template<class d> d* dup(d* x) { return x? x->duplicate() : NULL; }
  
  definition *definition::duplicate(remap_set &n) const {
    definition* res = new definition(name, parent, flags);
    n[this] = res;
    return res;
  }
  
  definition *definition_class::duplicate(remap_set &n) const {
    definition_class* res = new definition_class(name, parent, flags);
    res->definition_scope::copy(this, n);
    res->ancestors = ancestors;
    n[this] = res;
    return res;
  }
  
  definition *definition_enum::duplicate(remap_set &n) const {
    definition_enum* res = new definition_enum(name, parent, flags);
    res->type = type;
    res->constants.reserve(constants.size());
    for (vector<const_pair>::const_iterator c = constants.begin(); c != constants.end(); ++c)
      res->constants.push_back(const_pair(c->def, dup(c->ast)));
    n[this] = res;
    return res;
  }
  
  definition *definition_overload::duplicate(remap_set &n) const {
    ref_stack dup; dup.copy(referencers);
    definition_overload* res = new definition_overload(name, parent, type, dup, modifiers, flags);
    n[this] = res;
    return res;
  }
  
  definition *definition_function::duplicate(remap_set &n) const {
    definition_function *res = new definition_function(name, parent, flags);
    res->overloads = overloads;
    for (overload_iter it = res->overloads.begin(); it != res->overloads.end(); ++it)
      it->second = (definition_overload*)it->second->duplicate(n);
    n[this] = res;
    return res;
  }
  
  definition* definition_scope::duplicate(remap_set &n) const {
    definition_scope* res = new definition_scope(name, parent, flags);
    n[this] = res;
    
    res->copy(this, n);
    return res;
  }
  
  definition* definition_template::duplicate(remap_set &n) const {
    definition_template* res = new definition_template(name, parent, flags);
    n[this] = res;
    
    res->def = def->duplicate(n);
    res->specializations = specializations;
    res->instantiations = instantiations;
    res->params.reserve(params.size());
    for (pciterator it = params.begin(); it != params.end(); ++it)
      res->params.push_back((definition_tempparam*)(*it)->duplicate(n));
    for (speciter it = res->specializations.begin(); it != res->specializations.end(); ++it)
      for (speclist::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
        (*it2)->spec_temp = (definition_template*)(*it2)->spec_temp->duplicate(n);
    for (institer it = res->instantiations.begin(); it != res->instantiations.end(); ++it) {
      definition *nd = it->second->def->duplicate(n);
      n[it->second->def] = nd;
      it->second = new instantiation();
      it->second->def = nd;
    }
    return res;
  }
  
  definition* definition_tempparam::duplicate(remap_set &n) const {
    definition_tempparam* res = new definition_tempparam(name, parent, flags);
    n[this] = res;
    
    res->default_type.copy(default_type);
    res->default_value = default_value? default_value->duplicate() : NULL; // FIXME: This drops information! Need an AST::duplicate.
    res->definition_class::copy(this, n);
    return res;
  }
  
  definition* definition_typed::duplicate(remap_set &n) const {
    definition_typed* res = new definition_typed(name, parent, type, modifiers, flags);
    n[this] = res;
    
    res->referencers.copy(referencers);
    return res;
  }
  
  definition* definition_union::duplicate(remap_set &n) const {
    definition_union* res = new definition_union(name, parent, flags);
    n[this] = res;
    
    res->definition_scope::copy(this, n);
    return res;
  }
  
  definition* definition_atomic::duplicate(remap_set &) {
    return this;
  }
  
  definition* definition_hypothetical::duplicate(remap_set &n) const {
    definition_hypothetical* res = new definition_hypothetical(name, parent, flags, def->duplicate());
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
  
  void definition::remap(remap_set &n) {
    parent = filter(parent, n);
  }
  
  void definition_scope::remap(remap_set &n) {
    definition::remap(n);
    // cout << "Scope `" << name << "' has " << dec_order.size() << " ordered members" << endl;
    for (orditer it = dec_order.begin(); it != dec_order.end(); ++it) {
      definition *def = (*it)->def();
      remap_set::const_iterator ex = n.find(def);
      if (ex != n.end())
        dec_order.erase(it);
      else
        def->remap(n);
    }
    for (defiter it = members.begin(); it != members.end(); ++it) {
      remap_set::const_iterator ex = n.find(it->second);
      if (ex != n.end()) {
        delete it->second;
        it->second = ex->second;
      }
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
  
  void definition_class::remap(remap_set &n) {
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
  
  void definition_enum::remap(remap_set& n) {
    #ifdef DEBUG_MODE
    if (n.find(type) != n.end()) {
      cerr << "Why are you replacing `" << type->name << "'?" << endl;
    }
    #endif
    type = filter(type, n);
    
    for (vector<const_pair>::iterator it = constants.begin(); it != constants.end(); ++it) {
      definition_valued *d = filter(it->def, n);
      if (it->def == d) {
        it->def->remap(n);
        if (it->ast) { // FIXME: This doesn't keep counting after a successful remap!
          value cache = it->def->value_of;
          it->ast->remap(n);
          it->def->value_of = it->ast->eval();
          if (it->def->value_of.type == VT_DEPENDENT)
            cerr << "SHIT!" << endl;
          else {
            cout << "Refactored " << it->ast->toString() << ": " << cache.toString() << " → " << it->def->value_of.toString() << endl;
            delete it->ast;
            it->ast = NULL;
          }
          // cout << "Remap called on enum: new value of `" << it->def->name << "': " << cache.toString() << " => " << it->def->value_of.toString() << endl;
        }
      }
      else {
        delete it->def;
        it->def = d;
      }
    }
  }
  
  void definition_function::remap(remap_set& n) {
    definition::remap(n);
    for (overload_iter it = overloads.begin(); it != overloads.end(); ++it) {
      remap_citer rit = n.find(it->second);
      if (rit != n.end())
        it->second = (definition_overload*)rit->second;
      else
        it->second->remap(n);
    }
  }
  
  void definition_overload::remap(remap_set& n) {
    definition_typed::remap(n);
    // TODO: remap_function_implementation();
  }
  
  void definition_template::remap(remap_set &n) {
    if (def)
      def->remap(n);
  }
  
  void definition_tempparam::remap(remap_set &n) {
    // TODO: Implement
    (void)n;
  }
  
  void definition_typed::remap(remap_set &n) {
    remap_set::const_iterator ex = n.find(type);
    if (ex != n.end())
      type = ex->second;
  }
  
  void definition_union::remap(remap_set &) {
    
  }
  
  void definition_atomic::remap(remap_set &) {}
  
  void definition_hypothetical::remap(remap_set &n) {
    AST *nast = def->duplicate();
    nast->remap(n);
    full_type ft = nast->coerce();
    
    if (!ft.def || ft.def == arg_key::abstract) {
      cerr << "Problem instantiating `" + name + "': coercion failure in expression: " + nast->toString() << endl;
      #if 0 && defined(DEBUG_MODE)
      cerr << "Remap dump:" << endl;
      for (remap_citer rit = n.begin(); rit != n.end(); ++rit)
        cerr << (void*)rit->first << ": " << ((definition*)rit->first)->toString(2) << "\n=>\n" << (void*)rit->second << ": " << rit->second->toString(2) << endl; 
      cerr << "End dump." << endl;
      #endif
    }
    else {
      #ifdef DEBUG_MODE
        if (ft.refs.size() || ft.flags)
          cerr << "Coerced refstack somehow has additional info attached! Discarded!" << endl << endl << endl;
        // cout << "Successfully remapped " << name << " to" << endl << ft.def->toString(-1, 2) << endl;
      #endif
      n[(definition*)this] = ft.def;
    }
    delete nast;
  }
  
  void arg_key::remap(const remap_set &r) {
    for (node* n = values; n != endv; ++n)
      if (n->type == AKT_FULLTYPE) {
        n->ft().def = filter(n->ft().def, r);
        n->ft().reduce();
      }
      else if (n->av().ast) {
        AST *ao = n  -> av().ast;
        AST *a  = ao -> duplicate();
        a->remap(r);
        value v = a->eval();
        if (v.type != VT_DEPENDENT) {
          n->av().ast = NULL;
          n->val() = v;
          delete ao;
        }
        else {
          cerr << "No dice in unrolling template expression" << endl;
          cerr << "original expression: " << ao->toString() << endl;
          cerr << "evaluated expression: " << a->toString() << endl;
        }
        delete a;
      }
  }
  
  
  //========================================================================================================
  //======: AST Node Duplicate Functions :==================================================================
  //========================================================================================================
  
  AST::AST_Node *AST::AST_Node            ::duplicate() const { return new AST_Node(content, type);                         }
  AST::AST_Node *AST::AST_Node_Scope      ::duplicate() const { return new AST_Node_Scope(dup(left), dup(right), content);  }
  AST::AST_Node *AST::AST_Node_Unary      ::duplicate() const { return new AST_Node_Unary(dup(operand), content, prefix, AST_Node::type); }
  AST::AST_Node *AST::AST_Node_sizeof     ::duplicate() const { return new AST_Node_sizeof(dup(operand), negate);           }
  AST::AST_Node *AST::AST_Node_Definition ::duplicate() const { return new AST_Node_Definition(def, content);               }
  AST::AST_Node *AST::AST_Node_Type       ::duplicate() const { full_type dt(dec_type); return new AST_Node_Type(dt);       }
  AST::AST_Node *AST::AST_Node_Cast       ::duplicate() const { return new AST_Node_Cast(dup(operand), cast_type);          }
  AST::AST_Node *AST::AST_Node_Binary     ::duplicate() const { return new AST_Node_Binary(dup(left), dup(right), content); }
  AST::AST_Node *AST::AST_Node_Ternary    ::duplicate() const { return new AST_Node_Ternary(dup(exp), dup(left), dup(right), content); }
  AST::AST_Node *AST::AST_Node_new        ::duplicate() const { return new AST_Node_new(alloc_type, position, bound);       }
  AST::AST_Node *AST::AST_Node_delete     ::duplicate() const { return new AST_Node_delete(dup(operand), array);            }
  AST::AST_Node *AST::AST_Node_Subscript  ::duplicate() const { return new AST_Node_Subscript(dup(left), dup(index));       }
  
  AST::AST_Node *AST::AST_Node_Parameters ::duplicate() const {
    AST_Node_Parameters *res = new AST_Node_Parameters();
    res->func = dup(func);
    res->params.reserve(params.size());
    for (vector<AST_Node*>::const_iterator p = params.begin(); p != params.end(); ++p)
      res->params.push_back(dup(*p));
    return res;
  }
  AST::AST_Node *AST::AST_Node_TempInst   ::duplicate() const {
    AST_Node_TempInst *res = new AST_Node_TempInst(temp);
    res->params.reserve(params.size());
    for (vector<AST_Node*>::const_iterator p = params.begin(); p != params.end(); ++p)
      res->params.push_back(dup(*p));
    return res;
  }
  AST::AST_Node *AST::AST_Node_TempKeyInst::duplicate() const {
    return new AST_Node_TempKeyInst(temp, key);
  }
  AST::AST_Node *AST::AST_Node_Array      ::duplicate() const {
    AST_Node_Array *res = new AST_Node_Array();
    res->elements.reserve(elements.size());
    for (vector<AST_Node*>::const_iterator e = elements.begin(); e != elements.end(); ++e)
      res->elements.push_back(dup(*e));
    return res;
  }
  
  AST* AST::duplicate() const {
    return new AST(root->duplicate());
  }
  
  //========================================================================================================
  //======: AST Node Re-map Functions :=====================================================================
  //========================================================================================================
  
  template<class c> void nremap(c *x, const remap_set &n) {
    if (x) x->remap(n); else
      cerr << "Why is this null?" << endl;
    }
  
  void AST::AST_Node            ::remap(const remap_set&)   {  }
  void AST::AST_Node_Scope      ::remap(const remap_set& n) { AST_Node_Binary::remap(n);  }
  void AST::AST_Node_Unary      ::remap(const remap_set& n) { nremap(operand, n);         }
  void AST::AST_Node_sizeof     ::remap(const remap_set& n) { nremap(operand, n);         }
  void AST::AST_Node_Definition ::remap(const remap_set& n) { def = filter(def, n);       }
  void AST::AST_Node_Type       ::remap(const remap_set& n) { dec_type.def  = filter(dec_type.def,  n); }
  void AST::AST_Node_Cast       ::remap(const remap_set& n) { cast_type.def = filter(cast_type.def, n); }
  void AST::AST_Node_Binary     ::remap(const remap_set& n) { nremap(left, n); nremap(right, n);        }
  void AST::AST_Node_Ternary    ::remap(const remap_set& n) { nremap(left, n); nremap(right, n); nremap(exp, n); }
  void AST::AST_Node_new        ::remap(const remap_set& n) { alloc_type.def = filter(alloc_type.def, n); nremap(position, n); nremap(bound, n); }
  void AST::AST_Node_delete     ::remap(const remap_set& n) { AST_Node_Unary::remap(n);          }
  void AST::AST_Node_Subscript  ::remap(const remap_set& n) { nremap(left, n); nremap(index, n); }
  
  void AST::AST_Node_Parameters ::remap(const remap_set& n) {
    nremap(func, n);
    for (vector<AST_Node*>::iterator p = params.begin(); p != params.end(); ++p)
      (*p)->remap(n);
  }
  void AST::AST_Node_TempInst   ::remap(const remap_set& n) {
    temp = filter(temp, n);
    for (vector<AST_Node*>::iterator p = params.begin(); p != params.end(); ++p)
      (*p)->remap(n);
  }
  void AST::AST_Node_TempKeyInst::remap(const remap_set& n) {
    temp = filter(temp, n);
    key.remap(n);
  }
  void AST::AST_Node_Array      ::remap(const remap_set& n) {
    for (vector<AST_Node*>::iterator e = elements.begin(); e != elements.end(); ++e)
      (*e)->remap(n);
  }
  
}
