/**
 * @file  definition.h
 * @brief System header declaring structures for representing C definitions.
 * 
 * This file is likely used by absolutely everything in the parse system, as
 * it is the medium through which output definitions are created and manipulated.
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

#ifndef _DEFINITION__H
#define _DEFINITION__H

namespace jdi {
  /** Flags given to a definition to describe its type simply and quickly. **/
  enum DEF_FLAGS
  {
    DEF_TEMPLATE =     1 <<  0, ///< This definition has template parameters attached.
    DEF_TYPENAME =     1 <<  1, ///< This definition can be used as a typename. This does not imply that it has a valid type; see DEF_TYPED.
    DEF_NAMESPACE =    1 <<  2, ///< This definition is a namespace.
    DEF_CLASS =        1 <<  3, ///< This definition is a class or structure. 
    DEF_ENUM =         1 <<  4, ///< This definition is an enumeration of valued constants.
    DEF_UNION =        1 <<  5, ///< This definition is an enumeration of valued constants.
    DEF_SCOPE =        1 <<  6, ///< This definition is a scope of some sort.
    DEF_TYPED =        1 <<  7, ///< This definition contains a type and referencer list. Used with DEF_TYPENAME to mean TYPEDEF.
    DEF_FUNCTION =     1 <<  8, ///< This definition is a function containing a list of zero or more overloads.
    DEF_VALUED =       1 <<  9, ///< This definition has a constant integer value attached.
    DEF_DEFAULTED =    1 << 10, ///< This definition has a default expression attached.
    DEF_TEMPPARAM =    1 << 11, ///< This definition is a parameter of a template.
    DEF_EXTERN =       1 << 12, ///< This definition was declared with the "extern" flag.
    DEF_HYPOTHETICAL = 1 << 13, ///< This definition is a purely hypothetical template type, eg, template_param::typename type;
    DEF_PRIVATE =      1 << 14, ///< This definition was declared as a private member.
    DEF_PROTECTED =    1 << 15, ///< This definition was declared as a protected member.
    DEF_INCOMPLETE =   1 << 16, ///< This definition was declared but not implemented.
    DEF_ATOMIC =       1 << 17  ///< This is a global definition for objects of a fixed size, such as primitives.
  };
  
  struct definition;
  struct definition_typed;
  struct function_overload;
  struct definition_function;
  struct definition_valued;
  struct definition_scope;
  struct definition_class;
  struct definition_enum;
  struct definition_template;
  struct definition_atomic;
  struct definition_tempscope;
  struct definition_hypothetical;
}


#include <map>
#include <string>
#include <vector>
using namespace std;
typedef size_t pt;

namespace jdi {
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
    
    /// Map type to contain definitions to remap along with the definition with which it will be replaced
    typedef map<definition*, definition*> remap_set;
    
    /** Duplicate this definition, whatever it may contain.
        The duplicated version must be freed separately.
        @return A pointer to a newly-allocated copy of this definition. **/
    virtual definition* duplicate(remap_set &n);
    
    /** Re-map all uses of each definition used as a key in the remap_set to the
        corresponding definition used as the value. For example, if the given map
        contains { <builtin_type__float, builtin_type__double> }, then any float
        contained in this definition or its descendents will be replaced with a
        double. This can be used to eliminate references to a particular definition
        before free, or to instantiate templates without hassle. **/
    virtual void remap(const remap_set& n);
    
    /** Return the size of this definition. **/
    virtual size_t size_of();
    
    /** Compare two definitions, returning a comparison sign.
        @param d1  The first definition to compare.
        @param d2  The second definition to compare.
        @return  An integer sharing the sign of the comparison; positive if d1 > d2,
                 negative if d1 < d2, 0 if d1 == d2. */
    static ptrdiff_t defcmp(definition *d1, definition *d2);
    
    /** Print the contents of this scope to a string, returning it.
        @param levels  How many levels of children to print beneath this
                       scope, assuming this is a scope.
        @param indent  The indent, in spaces, to place before each line printed.
        @return Returns a string representation of everything in this definition.
    **/
    virtual string toString(unsigned levels = unsigned(-1), unsigned indent = 0);
    
    /** Construct a definition with a name, parent scope, and flags.
        Makes necessary allocations based on the given flags. **/
    definition(string n,definition* p,unsigned int f);
    /// Default constructor. Only valid for the global scope.
    definition();
    /// Default destructor.
    virtual ~definition();
  };
}

//=========================================================================================================
//===: Specializations with extended dependencies:=========================================================
//=========================================================================================================

#include <Storage/value.h>
#include <Storage/full_type.h>
#include <Storage/references.h>

namespace jdi {
  /**
    @struct jdi::definition_typed
    A piece of a definition for anything simply declared with a type.
    Use of this class includes regular variables, as well as typedefs. This is
    only part of the class for functions; see \c jdi::definition_function.
  **/
  struct definition_typed: definition {
    definition* type; ///< The definition of the type of this definition. This is not guaranteed to be non-NULL.
    ref_stack referencers; ///< Any referencers modifying the type, such as *, &, [], or (*)().
    unsigned int modifiers; ///< Flags such as long, const, unsigned, etc, as a bitmask. These can be looked up in \c builtin_decls_byflag.
    
    virtual definition* duplicate(remap_set &n);
    virtual void remap(const remap_set &n);
    virtual size_t size_of();
    virtual string toString(unsigned levels = unsigned(-1), unsigned indent = 0);
    
    /// Construct with all information. Consumes the given \c ref_stack.
    definition_typed(string name, definition* p, definition* tp, unsigned int typeflags, int flags = DEF_TYPED);
    definition_typed(string name, definition* p, definition* tp, ref_stack &rf, unsigned int typeflags, int flags = DEF_TYPED);
    
    virtual ~definition_typed();
  };
  /**
    @struct jdi::function_overload
    A structure detailing an alternate overload of a function without using too much memory.
  **/
  struct function_overload {
    full_type type; ///< The non-null return type of this overload.
    string declaration; ///< The full prototype for the function.
    function_overload *duplicate(); ///< Make a copy
  };
  /**
    @struct jdi::definition_function
    A piece of a definition specifically for functions.
    The class is based on implements a method of storing overload information.
  **/
  struct definition_function: definition_typed {
    virtual definition* duplicate(remap_set &n);
    virtual void remap(const remap_set &n);
    virtual size_t size_of();
    virtual string toString(unsigned levels = unsigned(-1), unsigned indent = 0);
    
    vector<function_overload*> overloads; ///< Array of reference stacks for each overload of this function.
    string declaration; ///< The full prototype for the function.
    void *implementation; ///< The implementation of this function as harvested by an external system.
    definition_function(string name, definition* p, definition* tp, ref_stack &rf, unsigned int typeflags, int flags = DEF_FUNCTION);
  };
  
  
  /**
    @struct jdi::definition_valued
    A subclass of definition for anything declared with a type, and given a value.
  **/
  struct definition_valued: definition_typed {
    value value_of; ///< The constant value of this definition.
    definition_valued(); ///< Default constructor; invalidates value.
    
    virtual string toString(unsigned levels = unsigned(-1), unsigned indent = 0);
    
    //definition_valued(string vname, definition *parnt, definition* type, unsigned int flags, value &val); ///< Construct with a value and type.
    definition_valued(string vname, definition *parnt, definition* type, unsigned int modifiers, unsigned int flags, value &val); ///< Construct with a value and type.
  };
  
  /**
    @struct jdi::definition_scope
    A piece of a definition for anything containing its own scope.
    This class is meant for namespaces, mostly. It is a base
    class for structs and classes; see \c jdi::definition_polyscope.
  **/
  struct definition_scope: definition {
    typedef map<string, definition*> defmap; ///< Shortcut defined to be the storage container by which definitions are looked up.
    typedef defmap::iterator defiter; ///< Shortcut to an iterator type for \c defmap.
    typedef defmap::const_iterator defiter_c; ///< Shortcut to a constant iterator type for \c defmap.
    typedef pair<defiter,bool> inspair; ///< The result from an insert operation on our map.
    typedef pair<string, definition*> entry; ///< The type of key-value entry pair stored in our map.
    
    defmap members; ///< Members of this enum or namespace
    defmap using_general; ///< A separate map of definitions to use
    
    /// Linked list node to contain using scopes
    struct using_node {
      definition_scope *use; ///< Scope to search
      private: friend class definition_scope;
        using_node *next; ///< The next node on our list, or NULL
        using_node(definition_scope* scope); ///< Construct with a scope to use
        using_node(definition_scope* scope, using_node* prev); ///< Construct with previous node
    };
    /** Add a namespace to the using list. This can technically be used on any scope. **/
    void use_namespace(definition_scope* scope);
    /** Add a namespace to the using list. This can technically be used on any scope. **/
    void use_general(string name, definition* def);
    
    /** Free all contents of this scope. No copy is made. **/
    void clear();
    /** Relinquish all content to another definition. **/
    void dump(definition_scope* to);
    /** Copy content from another definition. **/
    void copy(const definition_scope* from);
    /** Swap content with another definition. **/
    void swap(definition_scope* with);
    
    /** Look up a \c definition* given its identifier.
        @param name  The identifier by which the definition can be referenced. This is NOT qualified!
        @return  If found, a pointer to the definition with the given name is returned. Otherwise, NULL is returned.
    **/
    definition* look_up(string name);
    /** Look up a \c definition* in the current scope or its using scopes given its identifier.
        @param name  The identifier by which the definition can be referenced. This is NOT qualified!
        @return  If found, a pointer to the definition with the given name is returned. Otherwise, NULL is returned.
    **/
    definition* find_local(string name);
    
    virtual definition* duplicate(remap_set &n);
    virtual void remap(const remap_set &n);
    virtual size_t size_of();
    virtual string toString(unsigned levels = unsigned(-1), unsigned indent = 0);
    
    /** Default constructor. Only to be used for global! **/
    definition_scope();
    /** Overwrite copy constructor. **/
    definition_scope(const definition_scope&);
    /** Construct with a name and type.
        @param  name   The name of this scope.
        @param  parent The parent scope of this scope. Non-NULL (only global scope can have null parent).
        @param  flags  The type of this scope, such as DEF_NAMESPACE.
    **/
    definition_scope(string name, definition *parent, unsigned int flags);
    /** Default destructor, which simply calls clear().
        @warning If you reference objects that are local to a destroyed scope,
                 these references will be invalidated after that scope is destroyed.
    **/
    virtual ~definition_scope();
    
    private:
      /// First linked list entry
      using_node *using_front;
      /// Final linked list entry
      using_node *using_back;
  };
  
  /**
    @struct jdi::definition_class
    An extension of \c jdi::definition_scope for classes and structures, which can have ancestors.
  **/
  struct definition_class: definition_scope {
    virtual definition* duplicate(remap_set &n);
    virtual void remap(const remap_set &n);
    virtual size_t size_of();
    virtual string toString(unsigned levels = unsigned(-1), unsigned indent = 0);
    
    /// Simple structure for storing an inheritance type and the \c definition* of an ancestor.
    struct ancestor {
      unsigned protection; ///< The protection level of this inheritance, as one of the DEF_ constants, or 0 for public.
      definition_class* def; ///< The \c definition of the structure or class from which this one inherits members.
      ancestor(unsigned protection_level, definition_class* inherit_from); ///< Convenience constructor with both members.
      ancestor(); ///< Default constructor for vector.
    };
    vector<ancestor> ancestors; ///< Ancestors of this structure or class
    definition_class(string classname, definition_scope* parent, unsigned flags = DEF_CLASS | DEF_TYPENAME);
  };
  
  /**
    @struct jdi::definition_union
    An extension of \c jdi::definition_scope for unions, which have a unified tyoe and unique sizeof() operator.
  **/
  struct definition_union: definition_scope {
    virtual definition* duplicate(remap_set &n);
    virtual void remap(const remap_set &n);
    virtual size_t size_of();
    virtual string toString(unsigned levels = unsigned(-1), unsigned indent = 0);
    
    definition_union(string classname, definition_scope* parent, unsigned flags = DEF_CLASS | DEF_UNION | DEF_TYPENAME);
  };
  
  /**
    @struct jdi::definition_enum
    An extension of \c jdi::definition for enums, which contain mirrors of members in the parent scope.
  **/
  struct definition_enum: definition_typed {
    virtual definition* duplicate(remap_set &n);
    virtual void remap(const remap_set &n);
    virtual size_t size_of();
    virtual string toString(unsigned levels = unsigned(-1), unsigned indent = 0);
    
    definition_scope::defmap constants;
    definition_enum(string classname, definition_scope* parent, unsigned flags = DEF_ENUM | DEF_TYPENAME);
  };
  
  /**
    @struct jdi::definition_template
    A piece of a definition for anything with template parameters.
    This class can be used alongside structs, classes, and functions.
  **/
  struct definition_template: definition {
    /** The definition to which template parameters here contained are applied. **/
    definition* def;
    /** A list of template parameters or arguments, as typedefs.
        The definition pointed to by this vector must be a typedef.
        
        If the typedef is to NULL, the type is completely abstract. Otherwise,
        if the \c DEF_DEFAULTED flag is set, the type being pointed to is the
        default for this template parameter. If the type is non-null and this
        flag is not set, then this parameter has been specified in a (partial)
        instantiation of the parent function/class (the definition containing
        the vector).
    **/
    vector<definition*> params;
    
    virtual definition* duplicate(remap_set &n);
    virtual void remap(const remap_set &n);
    virtual size_t size_of();
    virtual string toString(unsigned levels = unsigned(-1), unsigned indent = 0);
    
    /** Structure containing template arguments; can be used as the key in an std::map. **/
    class arg_key {
      definition** values;
      // const unsigned sz;
      public:
        static definition abstract; ///< A sentinel pointer marking that this parameter is still abstract.
        /// A comparator to allow storage in a map.
        bool operator<(const arg_key& other) const;
        /// A method to prepare this instance for storage of parameter values for the given template.
        void mirror(definition_template* temp);
        /// A fast function to assign to our list at a given index.
        inline void put_final_type(size_t argnum, definition* type) { values[argnum] = type; }
        /// A slower function to put the most basic type representation down
        inline void put_type(size_t argnum, definition* type) {
          if (type->flags & DEF_TYPED) { put_type(argnum, ((definition_typed*)type)->type); return; }
          values[argnum] = type;
        }
        /// A quick function to grab the type at a position
        inline definition* operator[](int i) const { return values[i]; }
        /// A quick function to return an immutable pointer to the first parameter
        definition** begin() { return values; } 
        /// Default constructor; mark values NULL.
        inline arg_key(): values(NULL) {}
        /// Construct with a size, reserving sufficient memory.
        inline arg_key(size_t n): values(new definition*[n+1]) { *values = values[n] = 0; }
        /// Construct a copy.
        inline arg_key(const arg_key& other): values(other.values) { ((arg_key*)&other)->values = NULL; }
        /// Destruct, freeing items.
        inline ~arg_key() { if (values) { for (definition** i = values; *i; ++i) if (*i != &abstract) delete *i; delete[] values; } }
    };
    
    typedef map<arg_key,definition_template*> specmap; ///< Map type for specializations
    typedef specmap::iterator speciter; ///< Map iterator type for specializations
    
    typedef map<arg_key,definition*> instmap; ///< Map type for instantiations
    typedef instmap::iterator institer; ///< Map iterator type for instantiations
    
    /** A list of all specializations **/
    map<arg_key, definition_template*> specializations;
    /** A list of all existing instantiations **/
    map<arg_key, definition*> instantiations;
    
    /** Instantiate this template with the values given in the passed key.
        If this template has been instantiated previously, that instantiation is given.
        @param key  The \c arg_key structure containing the template parameter values to use. **/
    definition *instantiate(arg_key& key);
    /** Specialize this template with the values given in the passed key.
        If the specialization exists, it is returned. Otherwise, a new specialization
        is created with the flag DEF_INCOMPLETE to signify that it contains no unique definition.
        @param key  The \c arg_key structure containing the template parameter values to use.
        @param ts   The temporary scope allocated to store the template. **/
    definition_template *specialize(arg_key& key, definition_tempscope* ts);
    
    /** Construct with name, parent, and flags **/
    definition_template(string name, definition *parent, unsigned flags);
    /** Destructor to free template parameters, instantiations, etc. **/
    ~definition_template();
  };
  
  /**
    @struct jdi::definition_atomic
    A definition for atomic types.
  */
  struct definition_atomic: definition_scope {
    definition* duplicate(remap_set &n);
    void remap(const remap_set &n);
    size_t size_of();
    string toString(unsigned levels = unsigned(-1), unsigned indent = 0);
    size_t sz;
    definition_atomic(string n,definition* p,unsigned int f, size_t size);
  };
  
  /**
    @struct jdi::definition_tempscope
    A class which can be used as a temporary scope with a definition attached.
  */
  struct definition_tempscope: definition_scope {
    definition *source; ///< The definition for which this subscope was created.
    bool referenced; ///< Whether this s
    /** Construct with default information.
      @param name   Some unique key name for this scope.
      @param parent The scope above this one.
      @param flags  The additional flag data about this scope.
      @param source The definition with which this scope is affiliated, however loosely. */
    definition_tempscope(string name, definition* parent, unsigned flags, definition *source);
  };
  
  struct definition_hypothetical: definition {
    virtual definition* duplicate(remap_set &n);
    virtual void remap(const remap_set &n);
    virtual size_t size_of();
    virtual string toString(unsigned levels = unsigned(-1), unsigned indent = 0);
  };
}

#endif
