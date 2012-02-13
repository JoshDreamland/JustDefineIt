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

#ifdef DEBUG_MODE
#define track(ct) expression += ct + " "
#else
#define track(ct)
#endif

namespace jdi
{
  AST::AST_Node* AST::parse_expression(token_t &token, int prec_min) {
    if (token.type == TT_DECLARATOR or  token.type == TT_STRUCT
    or  token.type == TT_CLASS or  token.type == TT_ENUM or  token.type == TT_UNION)
    {
      //full_type ft = ?????->read_type(token); // Read complete type
      //if (token.type == TT_RIGHTPARENTH) // Check if we are a C-style cast (eg, (int)a)
      //  return new unary_cast(ft, parse_expression(token = lex->get_token(herr), prec_unary_postfix));
      //if (token.type == TT_LEFTPARENTH) // We could also be a C++ cast (eg, int(a))
      //  return new func_cast(ft, parse_expression(token = lex->get_token(herr)));
      token.report_error(herr, "Expected '(' or ')' after type name used in expression");
      return NULL;
    }
    AST_Node *myroot = parse_unary_pre_or_literal(token);
    if (!myroot) return NULL;
    myroot = parse_binary_or_unary_post(token,myroot,prec_min);
    return myroot;
  }
  AST::AST_Node* AST::parse_unary_pre_or_literal(token_t& token) {
    string ct;
    AST_Node *an = NULL;
    AST_TYPE at = AT_BINARYOP;
    bool read_next = false; // True at the end of this switch if the next token has already been read.
    switch (token.type)
    {
      case TT_DECLARATOR: case TT_CLASS: case TT_STRUCT: case TT_ENUM: case TT_UNION:
        token.report_error(herr, "Expected expression before declarator");
        return NULL;
      
      case TT_IDENTIFIER: an = new AST_Node(); an->content = string((const char*)token.extra.content.str,token.extra.content.len);
                          track(an->content); at = AT_IDENTIFIER; break;
      
      case TT_TYPENAME:
        token.report_error(herr, "Unimplemented.");
        return NULL;
      
      case TT_OPERATOR:
        ct = string((const char*)token.extra.content.str,token.extra.content.len);
        if (not(symbols[ct].type & ST_UNARY_PRE)) {
          token.report_error(herr,"Operator cannot be used as unary prefix");
          return NULL;
        }
        track(ct);
        token = lex->get_token();
        an = new AST_Node_Unary(parse_expression(token, 1000), ct);
        read_next = true;
        break;
      
      case TT_GREATERTHAN: case TT_LESSTHAN: case TT_COLON:
        token.report_error(herr, "Expected expression here before operator");
        return NULL;
      
      case TT_SCOPE:
      
      case TT_LEFTPARENTH:
        track(string("("));
        token = lex->get_token();
        an = parse_expression(token, 0);
        if (token.type != TT_RIGHTPARENTH) {
          token.report_error(herr, "Expected closing parenthesis at this point");
          return NULL;
        }
        track(string(")"));
        token = lex->get_token();
        return an;
      case TT_LEFTBRACKET:
      case TT_LEFTBRACE:
      case TT_TILDE: break;
      
      case TT_COMMA:
      case TT_SEMICOLON:
        token.report_error(herr, "Expected expression");
        return NULL;
      
      case TT_EQUALS:
      case TT_STRINGLITERAL:
      
      case TT_DECLITERAL: an = new AST_Node(); an->content = string((const char*)token.extra.content.str,token.extra.content.len);
                          track(an->content); at = AT_DECLITERAL; break;
      case TT_HEXLITERAL: an = new AST_Node(); an->content = string((const char*)token.extra.content.str,token.extra.content.len);
                          track(an->content); at = AT_HEXLITERAL; break;
      case TT_OCTLITERAL: an = new AST_Node(); an->content = string((const char*)token.extra.content.str,token.extra.content.len);
                          track(an->content); at = AT_OCTLITERAL; break;
      
      case TT_RIGHTPARENTH: case TT_RIGHTBRACKET: case TT_RIGHTBRACE: 
        return NULL; // Leave any error handling to caller.
      
      case TT_TEMPLATE: case TT_NAMESPACE: case TT_ENDOFCODE: case TT_TYPEDEF:
      case TT_USING: case TT_PUBLIC: case TT_PRIVATE: case TT_PROTECTED:
        token.report_error(herr, "Expected primary expression before %s token");
        return NULL;
      
      case TT_INVALID: default: token.report_error(herr, "Invalid token type returned!");
    }
    token_basics(
      an->type = at,
      an->filename = (const char*)token.file,
      an->linenum = token.linenum,
      an->pos = token.pos
    );
    if (!read_next)
      token = lex->get_token();
    return an;
  }
  AST::AST_Node* AST::parse_binary_or_unary_post(token_t &token, AST::AST_Node *left, int prec_min) {
    switch (token.type)
    {
      case TT_DECLARATOR: case TT_CLASS: case TT_STRUCT: case TT_ENUM: case TT_UNION:
        return left;
      
      case TT_IDENTIFIER:
      
      case TT_TYPENAME:
        token.report_error(herr, "Unimplemented.");
        return NULL;
      
      case TT_GREATERTHAN: if (!tt_greater_is_op) return left; 
      case TT_LESSTHAN: 
      case TT_COLON: return left;
      case TT_SCOPE: token.report_error(herr, "Unimplemented."); return NULL;
      
      case TT_OPERATOR: {
          string op((const char*)token.extra.content.str,token.extra.content.len);
          symbol &s = symbols[op];
          if (s.type & ST_BINARY) {
            if (s.prec < prec_min) return left;
            token = lex->get_token();
            track(op);
            AST_Node *right = parse_expression(token, s.prec + !(s.type & ST_RTL_PARSED));
            if (!right) {
              token.report_error(herr, "Expected secondary expression after binary operator");
              return left;
            }
            left = new AST_Node_Binary(left,right,op);
            break;
          }
          if (s.type & ST_TERNARY) {
            if (s.prec < prec_min) return left;
            string ct((const char*)token.extra.content.str,token.extra.content.len);
            track(ct);
            
            token = lex->get_token();
            AST_Node* exptrue = parse_expression(token, 0);
            if (!exptrue) return NULL;
            if (token.type != TT_COLON) { token.report_error(herr, "Colon expected"); return NULL; }
            track(string(":"));
            
            token = lex->get_token();
            AST_Node* expfalse = parse_expression(token, 0);
            if (!exptrue) return NULL;
            
            left = new AST_Node_Ternary(left,exptrue,expfalse,ct);
          }
        }
        break;
      case TT_TILDE:
      
      case TT_LEFTPARENTH:
      case TT_LEFTBRACKET:
      case TT_LEFTBRACE:
      case TT_COMMA:
      case TT_SEMICOLON:
      case TT_EQUALS:
      case TT_STRINGLITERAL:
      case TT_DECLITERAL:
      case TT_HEXLITERAL:
      case TT_OCTLITERAL:
      
      case TT_RIGHTPARENTH:
      case TT_RIGHTBRACKET:
      case TT_RIGHTBRACE:
      
      case TT_TEMPLATE: case TT_NAMESPACE: case TT_ENDOFCODE: case TT_TYPEDEF:
      case TT_USING: case TT_PUBLIC: case TT_PRIVATE: case TT_PROTECTED:
      return left;
      
      case TT_INVALID: default: return left;
    }
    return parse_binary_or_unary_post(token,left,prec_min);
  }
  
  int AST::parse_expression(lexer *ulex, error_handler *uherr) {
    lex = ulex, herr = uherr;
    token_t token = lex->get_token();
    root = parse_expression(token, 0);
    return 0;
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
  AST::AST_Node::AST_Node(string ct): parent(NULL), content(ct) {}
  AST::AST_Node_Unary::AST_Node_Unary(AST_Node* r): right(r) {}
  AST::AST_Node_Unary::AST_Node_Unary(AST_Node* r, string ct): AST_Node(ct), right(r) {}
  AST::AST_Node_Binary::AST_Node_Binary(AST_Node* l, AST_Node* r): left(l), right(r) {}
  AST::AST_Node_Binary::AST_Node_Binary(AST_Node* l, AST_Node* r, string op): AST_Node(op), left(l), right(r) {}
  AST::AST_Node_Ternary::AST_Node_Ternary(AST_Node *expression, AST_Node *exp_true, AST_Node *exp_false): exp(expression), left(exp_true), right(exp_false) {}
  AST::AST_Node_Ternary::AST_Node_Ternary(AST_Node *expression, AST_Node *exp_true, AST_Node *exp_false, string ct): AST_Node(ct), exp(expression), left(exp_true), right(exp_false) {}
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
