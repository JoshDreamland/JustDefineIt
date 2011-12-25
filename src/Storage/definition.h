/**
 * @file  definition.h
 * @brief System header declaring structures for representing C definitions.
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

#ifndef _DEFINITION__H
#define _DEFINITION__H

#include <map>
#include <string>
#include <vector>
using namespace std;
typedef size_t pt;

#include "references.h"

namespace jdi {
  enum DEF_FLAGS
  {
    DEF_TEMPLATE =     1 <<  0, ///< This definition has template parameters attached.
    DEF_TYPENAME =     1 <<  1, ///< This definition can be used as a typename. This does not imply that it has a valid type.
    DEF_NAMESPACE =    1 <<  2, ///< This definition is a namespace.
    DEF_CLASS =        1 <<  3, ///< This definition is a class or structure. 
    DEF_ENUM =         1 <<  4, ///< This definition is an enumeration of valued constants.
    DEF_TYPED =        1 <<  5, ///< This definition contains a type and referencer list.
    DEF_FUNCTION =     1 <<  6, ///< This definition is a function containing a list of zero or more overloads.
    DEF_TYPEDEF =      1 <<  7, ///< This type is aliased to another type; the type member exists but may be NULL.
    DEF_VALUED =       1 <<  8, ///< This definition has a constant integer value attached.
    DEF_DEFAULTED =    1 <<  9, ///< This definition has a default expression attached.
    DEF_TEMPPARAM =    1 << 10, ///< This definition is a parameter of a template.
    DEF_EXTERN =       1 << 11, ///< This definition was declared with the "extern" flag.
    DEF_HYPOTHETICAL = 1 << 12, ///< This definition is a purely hypothetical template type, eg, template_param::typename type;
    DEF_C99_STRUCT =   1 << 13  ///< This definition is a C99 structure.
    // DEF_PENDING_TYPE1 << 14DEF =   ///< Remove me.
  };
  
  struct definition_scope;
  
  /**
    @struct jdi::definition
    @brief  The class responsible for storing all parsed definitions.
    
    The class is geared to, at the cost of a couple pointers of size, be extensible. 
  **/
  struct definition
  {
    unsigned int flags; ///< DEF_FLAGS to use in identifying this definition's properties
    string name; ///< The name of this definition, as it appears in code.
    definition_scope* parent; ///< The definition of the scope in which this definition is declared.
                        ///< Except for the global scope of the context, this must be non-NULL.
    
    /** Duplicate this definition, whatever it may contain.
        The duplicated version must be freed separately.
        @return A pointer to a newly-allocated copy of this definition.
    **/
    virtual definition* duplicate();
    
    /** Construct a definition with a name, parent scope, and flags.
        Makes necessary allocations based on the given flags.
    **/
    definition(string n,definition* p,unsigned int f); 
    /// Default constructor. Only valid for the global scope.
    definition();
    /// Default destructor.
    virtual ~definition();
  };
  
  /**
    @struct jdi::definition_typed
    A piece of a definition for anything simply declared with a type.
    Use of this class includes regular variables, as well as typedefs. This is
    onle part of the class for functions; see \c jdi::definition_function.
  **/
  struct definition_typed: definition {
    definition* type; ///< The definition of the type of this definition. This is not guaranteed to be non-NULL.
    ref_stack referencers; ///< Any referencers modifying the type, such as *, &, [], or (*)().
    definition_typed(string name, definition* p, definition* tp, ref_stack rf);
  };
  /**
    @struct jdi::function_overload
    A structure detailing an alternate overload of a function without using too much memory.
  **/
  struct function_overload {
    definition *type; ///< The non-null return type of this overload.
    ref_stack  *referencers; ///< The complete stack of referencers modifying our result type.
    string      declaration; ///< The full prototype for the function.
  };
  /**
    @struct jdi::definition_function
    A piece of a definition specifically for functions.
    The class is based on implements a method of storing overload information.
  **/
  struct definition_function: definition_typed {
    function_overload *overloads;  ///< Array of reference stacks for each overload of this function.
  };
  
  
  /**
    @struct jdi::definition_constant
    A subclass of definition for anything simply declared with a type.
    This class includes regular variables, as well as typedefs. This is a
    base class for functions, as well; see \c jdi::definition_constant.
  **/
  struct definition_constant: definition {
    long value_of; ///< The constant value of this definition, which must be an integer.
  };
  
  /**
    @struct jdi::definition_scope
    A piece of a definition for anything containing its own scope.
    This class is meant for enums and namespaces, mostly. It is a base
    class for structs and classes; see \c jdi::definition_polyscope.
  **/
  struct definition_scope: definition {
    typedef map<string, definition*> defmap; ///< Shortcut defined to be the storage container by which definitions are looked up.
    typedef defmap::iterator defiter; ///< Shortcut to an iterator type for \c defmap.
    typedef pair<defiter,bool> inspair; ///< The result from an insert operation on our map.
    typedef pair<string, definition*> entry; ///< The type of key-value entry pair stored in our map.
    defmap members; ///< Members of this enum or namespace
    
    /** Free all contents of this scope. No copy is made. **/
    void clear();
    /** Relinquish all content to another definition. **/
    void dump(definition_scope* to);
    /** Copy content to another definition. **/
    void copy(definition_scope* to);
    /** Swap content with another definition. **/
    void swap(definition_scope* with);
    
    /** Look up a \c definition* given its identifier.
        @param   name  The identifier by which the definition can be referenced. This is NOT qualified!
        @return  If found, a pointer to the definition with the given name is returned. Otherwise, NULL is returned.
    **/
    definition* look_up(string name);
    
    definition* duplicate();
    
    /** Default constructor. Only to be used for global! **/
    definition_scope();
    /** Overwrite copy constructor. **/
    definition_scope(const definition_scope&);
    /** Construct with a name and type.
        @param  name   The name of this scope.
        @param  parent The parent scope of this scope. Non-NULL (except when constructing global scope).
        @param  flags  The type of this scope, such as DEF_NAMESPACE.
    **/
    definition_scope(string name, definition *parent, unsigned int flags);
    /** Default destructor, which simply calls clear().
        @warning If you reference objects that are local to a destroyed scope,
                 these references will be invalidated after that scope is destroyed.
    **/
    ~definition_scope();
  };
  /**
    @struct jdi::definition_polyscope
    An extension of \c jdi::definition_scope for classes and structures, which can have ancestors.
  **/
  struct definition_polyscope: definition_scope {
    vector<definition*> ancestors; ///< Ancestors of this structure or class
  };
  
  /**
    @struct jdi::definition_template
    A piece of a definition for anything with template parameters.
    This class can be used alongside structs, classes, and functions.
  **/
  struct definition_template {
    /** A list of template parameters or arguments, as typedefs.
        The definition pointed to by this vector must be a typedef.
        
        If the typedef is to NULL, the type is completely abstract. Otherwise,
        if the \c DEF_DEFAULTED flag is set, the type being pointed to is the
        default for this template parameter. If the type is non-null and this
        flag is not set, then this parameter has been specified in a (partial)
        instantiation of the parent function/class (the definition containing
        the vector).
    **/
    vector<definition*> tempargs;
  };
  
  /**
    @struct jdi::full_type
    A structure noting the \c definition associated with a type along with a
    set of modifier flags.
  **/
  struct full_type {
    jdi::definition *def; ///< The \c definition associated with the type
    jdi::ref_stack refs; ///< Any referencers affecting this type, such as the pointer-to asterisk (*) or ampersand reference (&).
    int flags; ///< Any flags, such as unsigned, signed, or const, associated with us, as a bitmask.
    
    full_type();
    full_type(jdi::definition*);
    full_type(jdi::definition*, jdi::ref_stack, int);
  };
}

#endif
