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

namespace jdi {
  definition *definition::duplicate() {
    return new definition(name, parent, flags);
  }
  definition::definition(string n,definition* p,unsigned int f): name(n), parent((definition_scope*)p), flags(f) {}
  definition::definition(): name(), parent(NULL), flags(0) {}
  definition::~definition() {}
  
  definition_typed::definition_typed(string name, definition* p, definition* tp, ref_stack rf): definition(name,p,DEF_TYPED), type(tp), referencers(rf) {}
  
  full_type::full_type(): def(NULL) {}
  full_type::full_type(jdi::definition* d): def(d) {}
  full_type::full_type(jdi::definition* d, jdi::ref_stack r, int f): def(d), refs(r), flags(f) {}
  
  definition *definition_scope::look_up(string name) {
    defiter it = members.find(name);
    if (it != members.end())
      return it->second;
    if (parent == NULL)
      return NULL;
    return parent->look_up(name);
  }
  void definition_scope::copy(definition_scope* to) {
    for (defiter it = members.begin(); it != members.end(); it++) {
      pair<defiter,bool> dest = to->members.insert(pair<string,definition*>(it->first,NULL));
      if (dest.second)
        dest.first->second = it->second->duplicate();
    }
  }
  definition* definition_scope::duplicate() {
    definition_scope* res = new definition_scope(name, parent, flags);
    copy(res);
    return res;
  }
  definition_scope::definition_scope(): definition() { }
  definition_scope::definition_scope(const definition_scope&): definition() { }
  definition_scope::definition_scope(string name, definition *parent, unsigned int flags): definition(name,parent,flags) {}
  definition_scope::~definition_scope() {
    for (defiter it = members.begin(); it != members.end(); it++)
      delete it->second;
    members.clear();
  }
}
