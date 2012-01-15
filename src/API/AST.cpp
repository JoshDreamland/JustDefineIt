/**
 * @file  AST.cpp
 * @brief Source implementing a class that handles expression evaluation and type
 *        coercion via an AST.
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

#include "AST.h"
#include <General/parse_basics.h>
#include <Storage/value_funcs.h>
#include <System/symbols.h>
#include <cstdio>
#include <map>

using namespace std;
using namespace jdip;

namespace jdi
{
  void AST::operator<<(jdip::token_t token)
  {
    AST_TYPE at;
    int precedence = 0, special_precedence = 0; bool rtl_parsed = 0;
    AST_Node* an;
    
    #ifdef DEBUG_MODE
    #define track(ct) expression += ct + " "
    #endif
    
    switch (token.type)
    {
      case TT_OPERATOR: {
        string ct = string((const char*)token.extra.content.str,token.extra.content.len);
        track(ct);
        symbol &s = symbols[ct];
        if (s.type & ST_RTL_PARSED)
          rtl_parsed = true;
        if (!current or current->type == AT_BINARYOP or current->type == AT_TERNARYOP or current->type == AT_UNARY_PREFIX) {
          if (!(s.type & ST_UNARY_PRE)) {
            return;
          }
          at = AT_UNARY_PREFIX;
          precedence = 3;
          an = new AST_Node_Unary();
        }
        else if (s.type & ST_UNARY_POST) {
          if (current->type != AT_DECLITERAL and current->type != AT_HEXLITERAL and current->type != AT_OCTLITERAL and current->type != AT_UNARY_POSTFIX) {
            return;
          }
          at = AT_UNARY_POSTFIX;
          precedence = 2;
          an = new AST_Node_Unary();
        }
        else {
          if (!(s.type & ST_BINARY)) {
            if (!(s.type & ST_TERNARY)) {
              return;
            }
            at = AT_TERNARYOP;
            precedence = s.prec;
            special_precedence = 9001;
            an = new AST_Node_Ternary();
          }
          else {
            at = AT_BINARYOP;
            precedence = s.prec;
            an = new AST_Node_Binary();
          }
        }
        an->content = ct;
      } break;
      
      case TT_DECLITERAL: an = new AST_Node(); an->content = string((const char*)token.extra.content.str,token.extra.content.len);
                          track(an->content); at = AT_DECLITERAL; break;
      case TT_OCTLITERAL: an = new AST_Node(); an->content = string((const char*)token.extra.content.str,token.extra.content.len);
                          track(an->content); at = AT_OCTLITERAL; break;
      case TT_HEXLITERAL: an = new AST_Node(); an->content = string((const char*)token.extra.content.str,token.extra.content.len);
                          track(an->content); at = AT_HEXLITERAL; break;
      
      case TT_DECLARATOR: break;
      case TT_IDENTIFIER: 
        an = new AST_Node(); an->content = string((const char*)token.extra.content.str,token.extra.content.len);
        track(an->content); at = AT_IDENTIFIER;
        break;
      
      case TT_COLON: // In this case, we need to traverse in search of a ternary operator.
          track(string(":"));
          while (!current or current->type != AT_TERNARYOP) {
            if (!current) {
              return;
            }
            current = current->parent;
          }
          if (!((AST_Node_Ternary*)current)->left or current->full()) {
            return;
          }
          ((AST_Node_Ternary*)current)->state = true;
        return;
      
      case TT_LEFTPARENTH:
          track(string("("));
          if (!current or current->type == AT_BINARYOP or current->type == AT_TERNARYOP or current->type == AT_UNARY_PREFIX) {
            precedence = 2;
            special_precedence = 9002;
            an = new AST_Node_Group();
            at = AT_OPEN_PARENTH;
          }
          else {
            an = new AST_Node_Parameters();
            at = AT_PARAMETER_START;
          }
        break;
      case TT_RIGHTPARENTH:
          track(string(")"));
          if (!current) {
            return;
          }
          while (current->type != AT_OPEN_PARENTH and current->type != AT_PARAMETER_START) {
            current = current->parent;
            if (!current) {
              return;
            }
          }
        return;
      case TT_LEFTBRACKET: break;
      case TT_RIGHTBRACKET: break;
      
      case TT_COMMA: break;
      
      case TT_EQUALS: break;
      
      case TT_SEMICOLON:
        cout << "Semicolon token not handled by higher system. sadface" << endl;
        return;
      case TT_CLASS: case TT_STRUCT: case TT_ENUM: case TT_UNION: case TT_NAMESPACE: 
      case TT_TEMPLATE: case TT_TYPENAME: case TT_TYPEDEF: case TT_USING: case TT_PUBLIC: case TT_PRIVATE: case TT_PROTECTED:
      case TT_SCOPE: case TT_LEFTBRACE: case TT_RIGHTBRACE: case TT_DESTRUCTOR: case TT_STRINGLITERAL: case TT_ENDOFCODE: case TT_INVALID: default:
        
        return;
    }
    
    token_basics(
      an->type = at,
      an->filename = (const char*)token.file,
      an->linenum = token.linenum,
      an->pos = token.pos
    );
    an->precedence = special_precedence? special_precedence : precedence;
    
    // Time to add it to our AST.
    // First, see if we have a root
    if (root == NULL) {
      root = current =  an; // We don't. This'll be a simple add.
      return;
    }
    
    // We now assume that since we already have at least one node, `current` is valid
    if (token.type == TT_OPERATOR and at != AT_UNARY_PREFIX) // If we're an operator (but not a unary prefix operator)
    { // Then we need to consume some other node in the AST.
      
      // The first thing to do is find an operator of higher or equal precedence and consume it.
      while (current->parent and current->parent->precedence < precedence)
        current = current->parent;
      
      // We only consume operators of equal precedence if we are left-to-right parsed, otherwise
      // we act as its child instead, replacing the root of its sub-AST.
      if (current->parent and !rtl_parsed and current->parent->precedence == precedence)
        current = current->parent;
      
      // We then consume the current node.
      if (current->parent)
        current->parent->setright(an);
      else
        root = an;
      an->parent = current->parent;
      
      an->setleft(current);
      current->parent = an;
      current = an;
    }
    else {
      if (current->full()) {
        return;
      }
      current->setright(an);
      an->parent = current;
      current = an;
    }
  }
  
  
  //===========================================================================================================================
  //=: Evaluators :============================================================================================================
  //===========================================================================================================================
  
  value AST::eval() {
    if (!root) return value();
    return root->eval();
  }
  
  value AST::AST_Node::eval() {
    if (type == AT_DECLITERAL) {
      bool is_float = false;
      while (is_letter(content[content.length()-1])) {
        if (content[content.length()-1] == 'f' or content[content.length()-1] == 'd')
          is_float = true;
        content.erase(content.length()-1);
      }
      if (!is_float)
      for (size_t i = 0; i < content.length(); i++)
        if (content[i] == '.')
          is_float = true;
      if (is_float)
        return value(atof(content.c_str()));
      return value(atol(content.c_str()));
    }
    return value();
  }
  value AST::AST_Node_Ternary::eval() {
    if (!exp) return value();
    value e = exp->eval();
    return value_boolean(e)? (left?left->eval():value()) : (right?right->eval():value());
  }
  value AST::AST_Node_Binary::eval() {
    if (!left or !right) return value();
    symbol_iter si = symbols.find(content);
    if (si == symbols.end()) return value();
    symbol &s = si->second;
    if (!s.operate) return value();
    value l = left->eval(), r = right->eval();
    value res = s.operate(l, r);
    //cout << l.val.i << " " << content << " " << r.val.i << " = " << res.val.i << endl;
    return res;
  }
  value AST::AST_Node_Unary::eval() {
    if (!right) { cout << "No operand to unary (operator" << content << ")!" << endl; return value(); }
    if (!symbols[content].operate_unary) { cout << "No method to unary (operator" << content << ")!" << endl; return value(); }
    value b4 = right->eval(), after = symbols[content].operate_unary(b4);
    //cout << content << "(" << b4.val.i << ") = " << after.val.i << endl;
    return after;
  }
  value AST::AST_Node_Group::eval() {
    return root?root->eval():value();
  }
  value AST::AST_Node_Parameters::eval() {
    return value(); // We can't evaluate a function call ;_;
  }
  
  
  //===========================================================================================================================
  //=: Constructors :==========================================================================================================
  //===========================================================================================================================
  
  AST::AST_Node::AST_Node(): parent(NULL) {}
  AST::AST_Node_Unary::AST_Node_Unary(): right(NULL) {}
  AST::AST_Node_Binary::AST_Node_Binary(): left(NULL), right(NULL) {}
  AST::AST_Node_Ternary::AST_Node_Ternary(): exp(NULL), left(NULL), right(NULL), state(false) {}
  AST::AST_Node_Group::AST_Node_Group(): root(NULL) {}
  AST::AST_Node_Parameters::AST_Node_Parameters(): func(NULL) {}
  
  
  //===========================================================================================================================
  //=: Destructors :===========================================================================================================
  //===========================================================================================================================
  
  AST::AST_Node::~AST_Node() { }
  AST::AST_Node_Binary::~AST_Node_Binary() { delete left; delete right; }
  AST::AST_Node_Unary::~AST_Node_Unary() { delete right; }
  AST::AST_Node_Ternary::~AST_Node_Ternary() { delete exp; delete left; delete right; }
  AST::AST_Node_Group::~AST_Node_Group() { delete root; }
  AST::AST_Node_Parameters::~AST_Node_Parameters() { for (size_t i = 0; i < params.size(); i++) delete params[i]; }
  
  
  //===========================================================================================================================
  //=: Virtual Setters :=======================================================================================================
  //===========================================================================================================================
  
  void AST::AST_Node::setleft(AST::AST_Node* n) { cout << "ERROR! Passed to wrong token type. DISCARDED TO PREVENT MEMORY LEAK." << endl; delete n; }
  void AST::AST_Node_Binary::setleft(AST::AST_Node* n) { if (n == this) for(;;); left = n;}
  void AST::AST_Node_Unary::setleft(AST::AST_Node* n) { right = n; }
  void AST::AST_Node_Ternary::setleft(AST::AST_Node* n) { exp = n; }
  void AST::AST_Node_Group::setleft(AST::AST_Node* n) { root = n; }
  void AST::AST_Node_Parameters::setleft(AST::AST_Node* n) { func = n; }
  
  void AST::AST_Node::setright(AST::AST_Node* n) { cout << "ERROR! Setting child node of a regular node {" << content << "}! DISCARDED TO PREVENT MEMORY LEAK." << endl; delete n; }
  void AST::AST_Node_Binary::setright(AST::AST_Node* n) { right = n;}
  void AST::AST_Node_Unary::setright(AST::AST_Node* n) { right = n; }
  void AST::AST_Node_Ternary::setright(AST::AST_Node* n) { if (!state) left = n; else right = n; }
  void AST::AST_Node_Group::setright(AST::AST_Node* n) { root = n; }
  void AST::AST_Node_Parameters::setright(AST::AST_Node* n) { params.push_back(n); }
  
  bool AST::AST_Node::full() { return false; }
  bool AST::AST_Node_Unary::full() { return right != NULL; }
  bool AST::AST_Node_Binary::full() { return right != NULL; }
  bool AST::AST_Node_Ternary::full() { return right != NULL; }
  bool AST::AST_Node_Group::full() { return root != NULL; }
  bool AST::AST_Node_Parameters::full() { return false; }
  
  
  //===========================================================================================================================
  //=: Everything else :=======================================================================================================
  //===========================================================================================================================
  
  void AST::clear() {
    delete root;
    #ifdef DEBUG_MODE
      expression.clear();
    #endif
    root = current = NULL;
  }
  
  AST::~AST() {
    delete root;
  }
}
