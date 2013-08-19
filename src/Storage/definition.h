/**
 * @file  definition.h
 * @brief System header declaring structures for representing C definitions.
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

#ifndef _DEFINITION__H
#define _DEFINITION__H

#include <General/quickreference.h>

namespace jdi {
  /** Flags given to a definition to describe its type simply and quickly. **/
  enum DEF_FLAGS
  {
    DEF_TYPENAME =     1 <<  0, ///< This definition can be used as a typename. This does not imply that it has a valid type; see DEF_TYPED.
    DEF_NAMESPACE =    1 <<  1, ///< This definition is a namespace.
    DEF_CLASS =        1 <<  2, ///< This definition is a class or structure. 
    DEF_ENUM =         1 <<  3, ///< This definition is an enumeration of valued constants.
    DEF_UNION =        1 <<  4, ///< This definition is a union of multiple types.
    DEF_SCOPE =        1 <<  5, ///< This definition is a scope of some sort.
    DEF_TYPED =        1 <<  6, ///< This definition contains a type and referencer list. Used with DEF_TYPENAME to mean TYPEDEF.
    DEF_FUNCTION =     1 <<  7, ///< This definition is a function containing a list of zero or more overloads.
    DEF_OVERLOAD =     1 <<  8, ///< This definition is a function overload, containing a type and implementation.
    DEF_VALUED =       1 <<  9, ///< This definition has a default expression attached.
    DEF_EXTERN =       1 << 10, ///< This definition was declared with the "extern" flag.
    DEF_TEMPLATE =     1 << 11, ///< This definition has template parameters attached.
    DEF_TEMPPARAM =    1 << 12, ///< This definition belongs to a list of template parameters, and is therefore abstract.
    DEF_HYPOTHETICAL = 1 << 13, ///< This definition is a purely hypothetical template type, eg, template_param::typename type;
    DEF_PRIVATE =      1 << 15, ///< This definition was declared as a private member.
    DEF_PROTECTED =    1 << 16, ///< This definition was declared as a protected member.
    DEF_INCOMPLETE =   1 << 17, ///< This definition was declared but not implemented.
    DEF_ATOMIC =       1 << 18  ///< This is a global definition for objects of a fixed size, such as primitives.
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
  struct definition_tempparam;
  struct definition_atomic;
  struct definition_hypothetical;
  
  /// Structure for inserting declarations into a scope.
  struct decpair {
    quick::double_pointer<definition> def; ///< The definition that was there previously, or that was inserted if it did not exist.
    bool inserted; ///< True if the given definition was inserted, false otherwise.
    /** Construct with data.
      @param def        A pointer to the pointer to the definition that exists under the previously given key.
      @param inserted   True if a new entry was created under the given key, false if the key-value was left unchanged.
    */
    decpair(definition* *def, bool inserted);
  };
}


#include <map>
#include <string>
#include <vector>
#include <deque>
#include <iostream>
using namespace std;
typedef size_t pt;

namespace jdi {
  /// Map type to contain definitions to remap along with the definition with which it will be replaced
  typedef map<const definition*, definition*> remap_set;
  typedef remap_set::const_iterator remap_citer;
  typedef remap_set::iterator remap_iter;
  
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
    
    virtual string kind() const; ///< Return the kind of this definition; an identifier for the class.
    
    /** Duplicate this definition, whatever it may contain.
        The duplicated version must be freed separately.
        @param n A remap_set containing any definitions to replace in this duplication.
                 This map will grow as more definitions are spawned recursively.
        @return A pointer to a newly-allocated copy of this definition. **/
    virtual definition* duplicate(remap_set &n) const;
    
    /** Re-map all uses of each definition used as a key in the remap_set to the
        corresponding definition used as the value. For example, if the given map
        contains { <builtin_type__float, builtin_type__double> }, then any float
        contained in this definition or its descendents will be replaced with a
        double. This can be used to eliminate references to a particular definition
        before free, or to instantiate templates without hassle. **/
    virtual void remap(remap_set& n);
    
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
    
    /// Return the qualified ID of this definition, eg, ::std::string.
    string qualified_id() const;
    
    #ifdef CUSTOM_MEMORY_MANAGEMENT
    void *operator new(size_t sz);
    void operator delete(void *ptr);
    #endif
    
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
#include <API/error_reporting.h>
#include <API/AST.h>

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
    
    virtual string kind() const;
    virtual definition* duplicate(remap_set &n) const;
    virtual void remap(remap_set &n);
    virtual size_t size_of();
    virtual string toString(unsigned levels = unsigned(-1), unsigned indent = 0);
    
    definition_typed(string name, definition* p, definition* tp, unsigned int typeflags, int flags = DEF_TYPED);
    /// Construct with all information. Consumes the given \c ref_stack.
    definition_typed(string name, definition* p, definition* tp, ref_stack *rf, unsigned int typeflags, int flags = DEF_TYPED);
    /// Construct without consuming a ref_stack.
    definition_typed(string name, definition* p, definition* tp, const ref_stack &rf, unsigned int typeflags, int flags = DEF_TYPED);
    
    virtual ~definition_typed();
  };
  
  /** Structure containing template arguments; can be used as the key in an std::map. **/
  class arg_key {
  public:
    enum ak_type { AKT_NONE, AKT_FULLTYPE, AKT_VALUE };
    /** Improvised C++ Union of full_type and value. */
    struct node {
      struct antialias {
        char data[
          (((sizeof(full_type) > sizeof(value))? sizeof(full_type) : sizeof(value)) + sizeof(char) - 1)
          /sizeof(char)
        ];
      } data;
      ak_type type;
      
      bool is_abstract() const;
      inline const full_type& ft() const { return *(full_type*)&data; }
      inline const value& val() const { return *(value*)&data; }
      inline full_type& ft() { return *(full_type*)&data; }
      inline value& val() { return *(value*)&data; }
      node &operator= (const node& other);
      bool operator!=(const node& x) const;
      
      inline node(): type(AKT_NONE) {}
      ~node();
    };
    
    private:
      /// An array of all our values
      node *values;
      /// A pointer past our value array
      node *endv;
      
    public:
      static definition abstract; ///< A sentinel pointer marking that this parameter is still abstract.
      /// A comparator to allow storage in a map.
      bool operator<(const arg_key& other) const;
      /// A method to prepare this instance for storage of parameter values for the given template.
      void mirror(definition_template* temp);
      /// Allocate a new definition for the parameter at the given index; this will be either a definition_typed or definition_valued.
      definition *new_definition(size_t index, string name, definition_scope* parent) const;
      /// A fast function to assign to our list at a given index, consuming the given type.
      void swap_final_type(size_t argnum, full_type &type);
      /// A less fast function to assign to our list at a given index, copying the given type.
      void put_final_type(size_t argnum, const full_type &type);
      /// A slower function to put the most basic type representation down, consuming the given type
      void swap_type(size_t argnum, full_type &type);
      /// An even slower function to put the most basic type representation down, copying the given starting type
      void put_type(size_t argnum, const full_type &type);
      /// Function to copy over a full node
      void put_node(size_t argnum, const node &n);
      /// A quick function to put a value at a given index
      void put_value(size_t argnum, const value& val);
      /// A quick function to grab the type at a position
      inline node &operator[](size_t x) { return values[x]; }
      inline const node &operator[](size_t x) const { return values[x]; }
      /// A quick function to return an immutable pointer to the first parameter
      inline node* begin() { return values; }
      /// A quick function to return a pointer past the end of our list
      inline node* end() { return endv; }
      /// Const begin() equivalent.
      inline const node* begin() const { return values; }
      /// Const end() equivalent.
      inline const node* end() const { return endv; }
      
      /// Return a string version of this key's argument list. You'll need to wrap in () or <> yourself.
      string toString() const;
      
      /// Default constructor; mark values NULL.
      arg_key();
      /// Construct with a size, reserving sufficient memory.
       arg_key(size_t n);
      /// Construct a copy.
      arg_key(const arg_key& other);
      /// Construct from a ref_stack.
      arg_key(const ref_stack& refs);
      /// Destruct, freeing items.
      ~arg_key();
  };
  
  /// Structure to augment an arg_key when choosing the correct specialization
  struct spec_key {
    /** This is an array of counts of uses of arguments in an arg_key in a template specialization.
        Each count is followed in memory by the indices of the parameters the argument was used in.
        For instance, this code:
        
           template<int a, int b, int c> struct x<a, a, b, c, b, b> { .. } 
        
        Produces this arg_inds:
        
           { {2, 0,1}, {1, 3}, {3, 2,4,5} }
        
        The argument `a' occurs two times: parameters 0 and 1.
        The argument `b' occurs one time: parameter 3.
        The argument `c' occurs three times: parameters 2, 4, and 5.
    */
    unsigned **arg_inds;
    /// The length of the arg_inds array.
    unsigned ind_count;
    /// The maximum number of parameters passed the same argument
    unsigned max_param;
    
    /** Calculates the merit of an arg_key to this spec_key; that is, returns how well it matches,
        which is based on the greatest number of parameter matches for one argument.
      @return Returns the calculated merit; 0 marks incompatibility. */
    int merit(const arg_key &k);
    /** Construct from a parameter count of a particular specialization, and the parameter count of the original.
        @param big_count    The parameter count of the specialization.
        @param small_count  The parameter count of the specialization.
    */
    spec_key(size_t big_count, size_t small_count);
    /* *
      Construct from a template's parameter list and an arg_key containing the specialization.
      @param special_template    The template whose parameters are used in the given key.
      @param specialization_key  The arg_key read in from the specialization's parameters.
                                 template<not this one> class my_specialized_class<this one> {};
    * /
    spec_key(const definition_template *special_template, const arg_key& specialization_key);*/
    /** Construct a new argument key from which to instantiate this specialization. */
    arg_key get_key(const arg_key &source_key);
    /// Compare to another key to check if they mean the same thing.
    bool same_as(const spec_key &other);
    /// Destruct, freeing array.
    ~spec_key();
    
    spec_key(const spec_key&, bool);
    private: spec_key(const spec_key&);
  };
  
  struct definition_overload: definition_typed {
    void *implementation; ///< The implementation of this function as harvested by an external system.
    
    virtual string kind() const;
    virtual definition* duplicate(remap_set &n) const;
    virtual void remap(remap_set &n);
    virtual string toString(unsigned levels = unsigned(-1), unsigned indent = 0);
    
    definition_overload(string name, definition* p, definition* tp, const ref_stack &rf, unsigned int typeflags, int flags = DEF_FUNCTION);
  };
  
  /**
    @struct jdi::definition_function
    A piece of a definition specifically for functions.
    The class is based on implements a method of storing overload information.
  **/
  struct definition_function: definition {
    typedef map<arg_key, definition_overload*> overload_map; ///< The map type used for storing overloads.
    typedef overload_map::iterator overload_iter; ///< An iterator type for \c overload_map.
    
    overload_map overloads; ///< Standard overloads, checked before template overloads.
    vector<definition_template*> template_overloads; ///< Array of reference stacks for each overload of this function.
    
    virtual string kind() const;
    virtual definition* duplicate(remap_set &n) const;
    virtual void remap(remap_set &n);
    virtual size_t size_of();
    virtual string toString(unsigned levels = unsigned(-1), unsigned indent = 0);
    
    /** Function to add the given definition as an overload if no such overload
        exists, or to merge it in (handling any errors) otherwise.
        @param tp        The full_type giving the return type and parameters.
        @param addflags  Any additional flags to be assigned to the overload.
        @param herr      Error handler to report problems to.
        @return Returns the final definition for the requested overload.
    */
    definition_overload *overload(const full_type &tp, unsigned addflags, error_handler *herr);
    definition_overload *overload(definition* tp, const ref_stack &rf, unsigned int typeflags, unsigned addflags, void *implementation, error_handler *herr);
    
    /** Function to add the given definition as a template overload
        exists, or to merge it in (handling any errors) otherwise.
        @param ovrl  The template definition representing the new overload; this definition
                     will belong to the system after you pass it.
    */
    void overload(definition_template* ovrl);
    
    definition_function(string name, definition* p, definition* tp, const ref_stack &rf, unsigned int typeflags, int flags = DEF_FUNCTION); ///< Create a function with one overload, created from the given ref_stack.
    definition_function(string name, definition* p, int flags = DEF_FUNCTION); ///< Create a function which has no prototypes/overloads; it is impossible to call this function.
    virtual ~definition_function();
  };
  
  
  /**
    @struct jdi::definition_valued
    A subclass of definition for anything declared with a type, and given a value.
  **/
  struct definition_valued: definition_typed {
    value value_of; ///< The constant value of this definition.
    definition_valued(); ///< Default constructor; invalidates value.
    
    virtual string kind() const;
    virtual string toString(unsigned levels = unsigned(-1), unsigned indent = 0);
    
    definition_valued(string vname, definition *parnt, definition* type, unsigned int modifiers, unsigned int flags, const value &val); ///< Construct with a value and type.
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
      using_node *next; ///< The next node on our list, or NULL.
      private: friend class definition_scope;
        using_node *prev; ///< The previous node on our list, or NULL, for removal purposes.
        using_node(definition_scope* scope); ///< Construct with a scope to use
        using_node(definition_scope* scope, using_node* prev); ///< Construct with previous node
    };
    /** Add a namespace to the using list. This can technically be used on any scope. **/
    using_node *use_namespace(definition_scope* scope);
    
    /// This is a map of structures declared in this scope, which is needed to emulate C's ability
    /// to have an object and structure by the same name. C++ extends this ability by allowing the
    /// behavior in any scope.
    map<string, definition*> c_structs;
    /** Function to insert into c_structs by the rules of definition_scope::declare.
        @param name  The name of the definition to declare.
        @param def   Pointer to the definition being declared, if one is presently available.
    */
    decpair declare_c_struct(string name, definition* def = NULL);
    
    /// A structure for keeping a pointer to a declaration.
    struct dec_order_g { virtual definition *def() = 0; virtual ~dec_order_g() {} };
    struct dec_order_defiter: dec_order_g {
      defiter it;
      dec_order_defiter(defiter i): it(i) {}
      virtual definition *def() { return it->second; }
      ~dec_order_defiter() {}
    };
    
    typedef deque<dec_order_g*> ordeque;
    typedef ordeque::iterator orditer;
    typedef ordeque::const_iterator orditer_c;
    
    /// A deque listing all declaraions (and dependent object references) in this scope, in order.
    /// May contain duplicates.
    ordeque dec_order;
    
    /** Remove a previously added namespace from our using list. **/
    void unuse_namespace(using_node *ns);
    /** Add a namespace to the using list. This can technically be used on any scope. **/
    void use_general(string name, definition* def);
    
    /** Free all contents of this scope. No copy is made. **/
    void clear();
    /** Relinquish all content to another definition. **/
    void dump(definition_scope* to);
    /** Copy content from another definition. **/
    void copy(const definition_scope* from, remap_set &n);
    /** Swap content with another definition. **/
    void swap(definition_scope* with);
    
    /** Look up a \c definition* given its identifier.
        @param name  The identifier by which the definition can be referenced. This is NOT qualified!
        @return  If found, a pointer to the definition with the given name is returned. Otherwise, NULL is returned.
    **/
    virtual definition* look_up(string name);
    /** Declare a definition by the given name in this scope. If no definition by that name exists in this scope,
        the given definition is inserted. Otherwise, the given definition is discarded, and the 
        @param name  The name of the definition to declare.
        @param def   A pointer to the definition in memory, or NULL if it will be allocated later.
        @return Returns a pair containing whether the item was inserted fresh and a mutable pointer to the data inserted.
                The pointer may be changed to indicate newly allocated data.
    **/
    virtual decpair declare(string name, definition* def = NULL);
    /** Look up a \c definition* in the current scope or its using scopes given its identifier.
        @param name  The identifier by which the definition can be referenced. This is NOT qualified!
        @return  If found, a pointer to the definition with the given name is returned. Otherwise, NULL is returned.
    **/
    virtual definition* find_local(string name);
    
    virtual string kind() const;
    virtual definition* duplicate(remap_set &n) const;
    virtual void remap(remap_set &n);
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
    
    protected:
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
    /// Simple structure for storing an inheritance type and the \c definition* of an ancestor.
    struct ancestor {
      unsigned protection; ///< The protection level of this inheritance, as one of the DEF_ constants, or 0 for public.
      definition_class* def; ///< The \c definition of the structure or class from which this one inherits members.
      ancestor(unsigned protection_level, definition_class* inherit_from); ///< Convenience constructor with both members.
      ancestor(); ///< Default constructor for vector.
    };
    
    /// Ancestors of this structure or class
    vector<ancestor> ancestors;
    
    virtual string kind() const;
    virtual definition* duplicate(remap_set &n) const;
    virtual void remap(remap_set &n);
    virtual size_t size_of();
    virtual string toString(unsigned levels = unsigned(-1), unsigned indent = 0);
    
    virtual definition* look_up(string name); ///< Look up a definition in this class (including its ancestors).
    virtual definition* find_local(string name);
    virtual decpair declare(string name, definition* def = NULL); ///< Declare a definition by the given name in this scope.
    
    definition_class(string classname, definition_scope* parent, unsigned flags = DEF_CLASS | DEF_TYPENAME);
  };
  
  /**
    @struct jdi::definition_union
    An extension of \c jdi::definition_scope for unions, which have a unified tyoe and unique sizeof() operator.
  **/
  struct definition_union: definition_scope {
    virtual string kind() const;
    virtual definition* duplicate(remap_set &n) const;
    virtual void remap(remap_set &n);
    virtual size_t size_of();
    virtual string toString(unsigned levels = unsigned(-1), unsigned indent = 0);
    
    definition_union(string classname, definition_scope* parent, unsigned flags = DEF_CLASS | DEF_UNION | DEF_TYPENAME);
  };
  
  /**
    @struct jdi::definition_enum
    An extension of \c jdi::definition for enums, which contain mirrors of members in the parent scope.
  **/
  struct definition_enum: definition_class {
    /// A small structure for storing definition_value definitions for each constant, plus an AST if the constant
    /// could not be evaluated at its occurrence (sometimes the case with templates).
    struct const_pair {
      definition_valued* def; ///< The definition generated for the constant.
      AST* ast; ///< If needed, an AST to allow re-evaluating the value expression later.
      const_pair(definition_valued *d, AST *a): def(d), ast(a) {} ///< Full constructor.
    };
    
    /// The constants contained in this enum, in order.
    vector<const_pair> constants;
    
    virtual string kind() const;
    virtual definition* duplicate(remap_set &n) const;
    virtual void remap(remap_set &n);
    virtual size_t size_of();
    virtual string toString(unsigned levels = unsigned(-1), unsigned indent = 0);
    
    definition *type; ///< The type of the constants in this enum; a cast to this type is valid but not favored (in overload resolution).
    unsigned modifiers; ///< Modifiers to our type, namely, unsigned.
    
    definition_enum(string classname, definition_scope* parent, unsigned flags = DEF_ENUM | DEF_TYPENAME);
    ~definition_enum();
  };
  
  /**
    @struct jdi::definition_template
    A piece of a definition for anything with template parameters.
    This class can be used alongside structs, classes, and functions.
  **/
  struct definition_template: definition_scope {
    /** The definition to which template parameters here contained are applied. **/
    definition* def;
    
    typedef vector<definition_tempparam*> pvector;
    typedef pvector::iterator piterator;
    typedef pvector::const_iterator pciterator;
    pvector params;
    
    struct specialization {
      spec_key key;
      definition_template *spec_temp;
      //specialization(definition_template *stemp, const arg_key &specialization_key):
      //  key(stemp, specialization_key), spec_temp(stemp) {}
      specialization(size_t big_count, size_t small_count, definition_template* spect):
        key(big_count, small_count), spec_temp(spect) {}
      specialization(const specialization&);
      ~specialization();
    };
    typedef vector<specialization*> speclist;
    typedef map<arg_key, speclist> specmap; ///< Map type for specializations
    typedef specmap::iterator speciter; ///< Map iterator type for specializations
    
    struct instantiation {
      definition* def;
      vector<definition*> parameter_defs;
      instantiation(): def(NULL), parameter_defs() {}
      ~instantiation();
    };
    
    typedef map<arg_key, instantiation> instmap; ///< Map type for instantiations
    typedef instmap::iterator institer; ///< Map iterator type for instantiations
    
    typedef vector<definition_hypothetical*> deplist; ///< Dependent member liat
    typedef deplist::iterator depiter; ///< Dependent member iterator
    
    struct dec_order_hypothetical: dec_order_g {
      definition_hypothetical *hyp;
      dec_order_hypothetical(definition_hypothetical *h): hyp(h) {}
      virtual definition *def();
      ~dec_order_hypothetical() {}
    };
    
    /** A map of all specializations **/
    specmap specializations;
    /** A map of all existing instantiations **/
    instmap instantiations;
    /** A map of all dependent members of our template parameters */
    deplist dependents;
    
    /** Instantiate this template with the values given in the passed key.
        If this template has been instantiated previously, that instantiation is given.
        @param key  The \c arg_key structure containing the template parameter values to use. **/
    definition *instantiate(arg_key& key, error_handler *herr);
    
    virtual string kind() const;
    virtual definition* duplicate(remap_set &n) const;
    virtual void remap(remap_set &n);
    virtual size_t size_of();
    virtual string toString(unsigned levels = unsigned(-1), unsigned indent = 0);
    
    /** Construct with name, parent, and flags **/
    definition_template(string name, definition *parent, unsigned flags);
    /** Destructor to free template parameters, instantiations, etc. **/
    ~definition_template();
  };
  
  /**
    @struct jdi::definition_tempparam
    A definition inheriting from definition_class, which is meant to represent a template parameter. Definitions in this
    class are considered hypothetical; they must exist in the type 
  */
  struct definition_tempparam: definition_class {
    AST *default_value;
    full_type default_type;
    bool must_be_class; ///< Denotes to the compiler that this template parameter was used in a way only a class can be used (such as inheritance or member access).
    
    /** Construct with default information.
      @param name   Some unique key name for this scope.
      @param parent The scope above this one.
      @param flags  The additional flag data about this scope. */
    definition_tempparam(string name, definition_scope* parent, unsigned flags = DEF_TEMPPARAM);
    /** Construct with a type and some flags. The DEF_TYPENAME flag is automatically given to the type, and the
        DEF_DEFAULTED flag is given to it if \p tp.def is non-null.
      @param name   Some unique key name for this scope.
      @param parent The scope above this one.
      @param tp     The default type given to this parameter.
      @param flags  The additional flag data about this scope. */
    definition_tempparam(string name, definition_scope* parent, full_type &tp, unsigned flags = DEF_TEMPPARAM | DEF_TYPENAME);
    /** Construct with default information.
      @param name   Some unique key name for this scope.
      @param parent The scope above this one.
      @param defval The default value given to this parameter, read in as an AST to enable it to depend on other parameters.
      @param flags  The additional flag data about this scope. */
    definition_tempparam(string name, definition_scope* parent, full_type &tp, AST* defval, unsigned flags = DEF_TEMPPARAM);
    
    virtual string kind() const;
    virtual definition* duplicate(remap_set &n) const;
    virtual void remap(remap_set &n);
    
    virtual definition* look_up(string name); ///< Look up a definition in the parent of this scope (skip this scope). This function will never be used by the system.
    virtual decpair declare(string name, definition* def = NULL); ///< Declare a definition by the given name in this scope. The definition will be marked HYPOTHETICAL, and the \c must_be_class flag will be set.
    virtual definition* find_local(string name); ///< Behaves identically to declare if the given name does not exist, or else returns it. In either case, the returned definition will be HYPOTHETICAL.
    
    ~definition_tempparam();
  };
  
  /**
    @struct jdi::definition_atomic
    A definition for atomic types.
  */
  struct definition_atomic: definition_scope {
    definition* duplicate(remap_set &n);
    
    string kind() const;
    void remap(remap_set &n);
    size_t size_of();
    string toString(unsigned levels = unsigned(-1), unsigned indent = 0);
    size_t sz;
    definition_atomic(string n,definition* p,unsigned int f, size_t size);
  };
  
  /**
    @struct jdi::definition_hypothetical
    A class representing a dependent (here called "hypothetical") type--a type which
    depends on an abstract parent or scope.
  */
  struct definition_hypothetical: definition_class {
    AST *def;
    unsigned int required_flags; ///< Set of flags required of any type provided to this template parameter
    
    virtual string kind() const;
    virtual definition* duplicate(remap_set &n) const;
    virtual void remap(remap_set &n);
    virtual size_t size_of();
    virtual string toString(unsigned levels = unsigned(-1), unsigned indent = 0);
    
    /// Construct with basic definition info.
    definition_hypothetical(string name, definition_scope *parent, unsigned flags, AST* def);
    definition_hypothetical(string name, definition_scope *parent, AST* def);
    ~definition_hypothetical();
  };
  
  
  //==========================================================================================
  //===: User Definitions :===================================================================
  //==========================================================================================
  
  // These definitions are not used by JDI, but rather are provided as utility classes for the
  // end-user. Use them if you want. Or don't.
  
  /**
    A smart scope designed to be used (as in a using directive) by another scope.
    This class automatically adds itself to the using collection of the given scope
    on construct, and automatically removes itself on destruct.
  */
  struct using_scope: definition_scope {
    jdi::definition_scope::using_node* using_me; ///< The scope that is using us.
    /// Construct with name and user scope.
    /// @param name  The name of this scope, in case a trace is ever printed.
    /// @param user  The scope which will use this scope, as in a using directive.
    using_scope(string name, definition_scope* user);
    ~using_scope();
  };
}

#endif
