/**
 * @file  AST.h
 * @brief Header declaring a class that handles expression evaluation and type
 *        coercion via an AST.
 * 
 * This file fuels "private" parser methods, but is designed to be a general-purpose
 * utility which is safe for use by typical JDI users as well. Simply pass it tokens
 * you create with the methods in \file user_tokens.h.
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
 * 
**/

#include <string>
#include <System/token.h>
#include <Storage/value.h>

namespace jdi {
  /** @class jdi::AST
      General-purpose class designed to take in a series of tokens and generate an abstract syntax tree.
      The generated AST can then be evaluated for a \c value or coerced for a resultant type as a \c definition.
  **/
  class AST
  {
    #ifdef DEBUG_MODE
    string expression; ///< The string representation of the expression fed in, for debug purposes.
    #endif
    
    /** Enum declaring basic node types for this AST. These include the three types of
        operators and the four types of data.
    **/
    enum AST_TYPE {
      AT_UNARY_PREFIX,
      AT_UNARY_POSTFIX,
      AT_BINARYOP,
      AT_TERNARYOP,
      AT_DECLITERAL,
      AT_HEXLITERAL,
      AT_OCTLITERAL,
      AT_IDENTIFIER,
      AT_OPEN_PARENTH,
      AT_PARAMETER_START
    };
    
    /// This class can export SVG files. Some info needs tossed around to do so.
    struct SVGrenderInfo;
    
    /** Private storage mechanism designed to hold token information and any linkages.
        In general, a node has no linkages, and so we use AST_Node as the base class for
        nodes which do have links.
    **/
    struct AST_Node {
      AST_TYPE type; ///< The type of this node, as one of the AST_TYPE constants.
      AST_Node *parent; ///< The parent of this node, or NULL iff it is the root.
      std::string content; ///< The literal, as a string, such as "1234", or the symbol representing the operator, as a symbol for lookup, such as "+=".
      int precedence; ///< The priority with which this node is executed; 0 = highest precedence.
      
      #ifndef NO_ERROR_REPORTING
        std::string filename; ///< The name of the file in which the token was created, for error reporting.
        int linenum; ///< The line on which this token appeared in the file.
        #ifndef NO_ERROR_POSITION
          int pos; ///< The position at which this token appeared in the line.
        #endif
      #endif
      
      /// Evaluates this node recursively, returning a value containing its result.
      virtual value eval();
      
      AST_Node(); ///< Default constructor.
      virtual ~AST_Node(); ///< Virtual destructor.
      virtual void setleft(AST_Node* l); ///< Assign to the left node, if there is one. Otherwise, sets error state.
      virtual void setright(AST_Node* r); ///< Assign to the right node, if there is one and it is empty. Otherwise, sets error state.
      virtual bool full(); ///< Returns true if this node is already completely full, meaning it has no room for children.
      
      virtual void print(); ///< Prints the contents of this node to stdout, recursively.
      virtual void toSVG(int x, int y, SVGrenderInfo* svg); ///< Renders this node and its children as an SVG.
      virtual int own_width(); ///< Returns the width in pixels of this node as it will render. This does not include its children.
      virtual int width(); ///< Returns the width which will be used to render this node and all its children.
      virtual int height(); ///< Returns the height which will be used to render this node and all its children.
    };
    /// Child of AST_Node for unary operators.
    struct AST_Node_Unary: AST_Node {
      AST_Node *right; ///< The stuff we're operating on.
      
      /// Evaluates this node recursively, returning a value containing its result.
      value eval();
      
      AST_Node_Unary(); ///< Default constructor. Sets children to NULL.
      ~AST_Node_Unary(); ///< Default destructor. Frees children recursively.
      void setleft(AST_Node* l); ///< Set the operand (same as \c setright).
      void setright(AST_Node* r); ///< Set the operand (same as \c setleft).
      bool full(); ///< Returns true if this node is already completely full, meaning it has no room for children.
      
      void print(); ///< Prints the contents of this node to stdout, recursively.
      void toSVG(int x, int y, SVGrenderInfo* svg); ///< Renders this node and its children as an SVG.
      int width(); ///< Returns the width which will be used to render this node and all its children.
      int height(); ///< Returns the height which will be used to render this node and all its children.
    };
    /// Child of AST_Node for binary operators.
    struct AST_Node_Binary: AST_Node {
      AST_Node *left, ///< The left-hand side of the expression.
               *right; ///< The right-hand side of the expression.
      
      /// Evaluates this node recursively, returning a value containing its result.
      value eval();
      
      AST_Node_Binary(); ///< Default constructor. Sets children to NULL.
      ~AST_Node_Binary(); ///< Default destructor. Frees children recursively.
      void setleft(AST_Node* l); ///< Set the left-hand operand.
      void setright(AST_Node* r); ///< Set the right-hand operand.
      bool full(); ///< Returns true if this node is already completely full, meaning it has no room for children.
      
      void print(); ///< Prints the contents of this node to stdout, recursively.
      void toSVG(int x, int y, SVGrenderInfo* svg); ///< Renders this node and its children as an SVG.
      int width(); ///< Returns the width which will be used to render this node and all its children.
      int height(); ///< Returns the height which will be used to render this node and all its children.
    };
    /// Child of AST_Node for the ternary operator.
    struct AST_Node_Ternary: AST_Node {
      AST_Node *exp, ///< The expression before the question marks.
               *left, ///< The left-hand (true) result.
               *right; ///< The right-hand (false) result.
      
      bool state; ///< Unfortunately, we need to store state information to allow passive parsing.
      
      /// Evaluates this node recursively, returning a value containing its result.
      value eval();
      
      AST_Node_Ternary(); ///< Default constructor. Sets children to NULL.
      ~AST_Node_Ternary(); ///< Default destructor. Frees children recursively.
      void setleft(AST_Node* l); ///< Set the left-hand operand (the test expression).
      void setright(AST_Node* r); ///< Set the right-hand operands (the first empty result).
      bool full(); ///< Returns true if this node is already completely full, meaning it has no room for children.
      
      void print(); ///< Prints the contents of this node to stdout, recursively.
      void toSVG(int x, int y, SVGrenderInfo* svg); ///< Renders this node and its children as an SVG.
      int width(); ///< Returns the width which will be used to render this node and all its children.
      int height(); ///< Returns the height which will be used to render this node and all its children.
    };
    /// Child of AST_Node for parenthetical groupings. @deprecated
    struct AST_Node_Group: AST_Node {
      AST_Node *root;
      
      /// Evaluates this node recursively, returning a value containing its result.
      value eval();
      
      AST_Node_Group(); ///< Default constructor. Sets children to NULL.
      ~AST_Node_Group(); ///< Default destructor. Frees children recursively.
      void setleft(AST_Node* l); ///< Set the operand (the nested expression).
      void setright(AST_Node* r); ///< Set the operand (the nested expression).
      bool full(); ///< Returns true if this node is already completely full, meaning it has no room for children.
      
      void print(); ///< Prints the contents of this node to stdout, recursively.
      void toSVG(int x, int y, SVGrenderInfo* svg); ///< Renders this node and its children as an SVG.
      int width(); ///< Returns the width which will be used to render this node and all its children.
      int own_width(); ///< Returns the width of this individual node, regardless of children.
      int height(); ///< Returns the height which will be used to render this node and all its children.
    };
    /// Child of AST_Node for array subscripts.
    struct AST_Node_Subscript: AST_Node {
      AST_Node *left;
      AST_Node *index;
      
      /// Evaluates this node recursively, returning a value containing its result.
      value eval();
      
      AST_Node_Subscript(); ///< Default constructor. Sets children to NULL.
      ~AST_Node_Subscript(); ///< Default destructor. Frees children recursively.
      void setleft(AST_Node* l); ///< Set the left-hand operand (the container).
      void setright(AST_Node* r); ///< Set the right-hand operand (the index expression).
      bool full(); ///< Returns true if this node is already completely full, meaning it has no room for children.
      
      void print(); ///< Prints the contents of this node to stdout, recursively.
      void toSVG(int x, int y, SVGrenderInfo* svg); ///< Renders this node and its children as an SVG.
      int width(); ///< Returns the width which will be used to render this node and all its children.
      int height(); ///< Returns the height which will be used to render this node and all its children.
    };
    /// Child of AST_Node for function call parameters.
    struct AST_Node_Parameters: AST_Node {
      AST_Node *func;
      vector<AST_Node*> params;
      
      /// Evaluates this node recursively, returning a value containing its result.
      value eval();
      
      AST_Node_Parameters(); ///< Default constructor. Sets children to NULL.
      ~AST_Node_Parameters(); ///< Default destructor. Frees children recursively.
      void setleft(AST_Node* l); ///< Set the left-hand operand (the function).
      void setright(AST_Node* r); ///< Set the right-hand operand (adds a parameter).
      bool full(); ///< Returns true if this node is already completely full, meaning it has no room for children.
      
      void print(); ///< Prints the contents of this node to stdout, recursively.
      void toSVG(int x, int y, SVGrenderInfo* svg); ///< Renders this node and its children as an SVG.
      int width(); ///< Returns the width which will be used to render this node and all its children.
      int height(); ///< Returns the height which will be used to render this node and all its children.
    };
    
    AST_Node *root; ///< The first node in our AST--The last operation that will be performed.
    AST_Node *current; ///< A buffer containing the tokens to be lexed.
    
    public:
    
    /// Push a token into the AST.
    void operator<<(jdip::token_t token);
    
    /// Evaluate the current AST, returning its \c value.
    value eval();
    
    /// Coerce the current AST for the type of its result.
    definition coerce();
    
    /// Clear the AST out, effectively creating a new instance of this class
    void clear();
    
    /// Print the AST to stdout.
    void print();
    
    /// Render the AST to an SVG file.
    void writeSVG(const char* filename);
    
    /// Default constructor. Zeroes some stuff.
    AST();
    
    /// Default destructor. Deletes the AST.
    ~AST();
  };
}

