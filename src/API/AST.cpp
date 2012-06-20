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
#include <System/builtins.h>
#include <System/symbols.h>
#include <Parser/bodies.h>
#include <API/compile_settings.h>
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
    string ct;
    AST_Node *myroot = NULL;
    AST_TYPE at = AT_BINARYOP;
    
    bool read_next = false; // True at the end of this switch if the next token has already been read.
    bool handled_basics = false; // True at the end of this switch if the basic token info was already read in.
    switch (token.type)
    {
      case TT_DECLARATOR: case TT_DECFLAG: case TT_CLASS: case TT_STRUCT: case TT_ENUM: case TT_UNION: case TT_EXTERN: {
          full_type ft = read_type(lex,token,search_scope,NULL,herr); // Read complete type
          myroot = new AST_Node_Type(ft);
          token_basics(
            myroot->type = AT_TYPE,
            myroot->filename = (const char*)token.file,
            myroot->linenum = token.linenum,
            myroot->pos = token.pos
          );
          if (token.type == TT_RIGHTPARENTH) // Facilitate casts
            return myroot;
          handled_basics = read_next = true;
        } break;
      
      case TT_IDENTIFIER: case TT_DEFINITION: {
          if (search_scope) {
            string n(token.content.toString());
            definition *def = search_scope->look_up(n);
            myroot = new AST_Node_Definition(def);
            at = AT_DEFINITION;
          }
          else {
            myroot = new AST_Node();
            at = AT_IDENTIFIER;
          }
          myroot->content = string(token.content.toString());
          track(myroot->content);
        } break;
      
      case TT_OPERATORKW:
        token.report_error(herr,"Please refer to operators in their binary format; explicit use of operator functions not presently supported.");
        break;
      
      case TT_TYPENAME:
        token.report_error(herr, "Unimplemented.");
        return NULL;
      
      case TT_OPERATOR: case TT_TILDE:
        ct = string(token.content.toString());
        if (not(symbols[ct].type & ST_UNARY_PRE)) {
          token.report_error(herr,"Operator cannot be used as unary prefix");
          return NULL;
        }
        track(ct);
        token = get_next_token();
        myroot = new AST_Node_Unary(parse_expression(token, PRECEDENCE_MAX), ct);
        read_next = true;
        break;
      
      case TT_GREATERTHAN: case TT_LESSTHAN: case TT_COLON:
        token.report_error(herr, "Expected expression here before operator");
        return NULL;
      
      case TT_SCOPE: break;
      
      case TT_LEFTPARENTH:
        track(string("("));
        token = get_next_token();
        myroot = parse_expression(token, 0);
        if (myroot == NULL) return NULL;
        if (token.type != TT_RIGHTPARENTH) {
          token.report_errorf(herr, "Expected closing parenthesis here before %s");
          return NULL;
        }
        track(string(")"));
        if (myroot->type == AT_TYPE) {
          AST_Node_Type *ad = (AST_Node_Type*)myroot;
          token = get_next_token(); read_next = true;
          AST_Node_Cast *nr = new AST_Node_Cast(parse_expression(token, PRECEDENCE_MAX));
          nr->cast_type.swap(ad->dec_type); nr->content = nr->cast_type.def->name;
          delete myroot; myroot = nr;
        }
        handled_basics = true;
        break;
      case TT_LEFTBRACKET:
      case TT_LEFTBRACE:
      break;
      
      case TT_COMMA:
      case TT_SEMICOLON:
        token.report_error(herr, "Expected expression");
        return NULL;
      
      case TT_STRINGLITERAL:
      case TT_CHARLITERAL: myroot = new AST_Node(); myroot->content = string(token.content.toString());
                           track(myroot->content); at = AT_CHRLITERAL; break;
      
      case TT_DECLITERAL: myroot = new AST_Node(); myroot->content = string(token.content.toString());
                          track(myroot->content); at = AT_DECLITERAL; break;
      case TT_HEXLITERAL: myroot = new AST_Node(); myroot->content = string(token.content.toString());
                          track(myroot->content); at = AT_HEXLITERAL; break;
      case TT_OCTLITERAL: myroot = new AST_Node(); myroot->content = string(token.content.toString());
                          track(myroot->content); at = AT_OCTLITERAL; break;
      
      case TT_DECLTYPE:
        
        break;
      
      case TT_SIZEOF:
          token = get_next_token(); track(string("sizeof")); 
          if (token.type == TT_LEFTPARENTH) {
              token = get_next_token(); track(string("(")); 
              myroot = new AST_Node_sizeof(parse_expression(token,PRECEDENCE_MAX));
              if (token.type != TT_RIGHTPARENTH)
                token.report_errorf(herr, "Expected closing parenthesis to sizeof before %s");
              else { track(string("(")); }
              token = get_next_token();
          }
          else
            myroot = new AST_Node_sizeof(parse_expression(token,PRECEDENCE_MAX));
          at = AT_UNARY_PREFIX;
          read_next = true;
        break;
      
      case TT_ELLIPSIS:
      case TT_RIGHTPARENTH: case TT_RIGHTBRACKET: case TT_RIGHTBRACE:
        // Overflow; same error.
      case TT_TEMPLATE: case TT_NAMESPACE: case TT_ENDOFCODE: case TT_TYPEDEF: case TT_ASM:
      case TT_USING: case TT_PUBLIC: case TT_PRIVATE: case TT_PROTECTED: 
        token.report_errorf(herr, "Expected expression before %s");
        return NULL;
      
      case TTM_CONCAT: case TTM_TOSTRING: token.report_error(herr, "Illogical token type returned!"); break;
      case TT_INVALID: default: token.report_error(herr, "Invalid token type returned!");
    }
    if (!handled_basics)
      token_basics(
        myroot->type = at,
        myroot->filename = (const char*)token.file,
        myroot->linenum = token.linenum,
        myroot->pos = token.pos
      );
    if (!read_next)
      token = get_next_token();
    
    myroot = parse_binary_or_unary_post(token,myroot,prec_min);
    return myroot;
  }
  
  AST::AST_Node* AST::parse_binary_or_unary_post(token_t &token, AST::AST_Node *left_node, int prec_min) {
    switch (token.type)
    {
      case TT_DECLARATOR: case TT_DECFLAG: case TT_CLASS: case TT_STRUCT: case TT_ENUM: case TT_UNION: case TT_EXTERN:
        return left_node;
      
      case TT_IDENTIFIER: case TT_DEFINITION:
      
      case TT_TYPENAME:
        token.report_error(herr, "Unimplemented.");
        return NULL;
      
      case TT_COLON:
        return left_node;
      
      case TT_SCOPE: token.report_error(herr, "Unimplemented."); return NULL;
      
      case TT_GREATERTHAN:
        if (!tt_greater_is_op)
          return left_node; 
      case TT_LESSTHAN:
      case TT_OPERATOR: {
          string op(token.content.toString());
          symbol &s = symbols[op];
          if (s.type & ST_BINARY) {
            if (s.prec < prec_min) return left_node;
            token = get_next_token();
            track(op);
            AST_Node *right = parse_expression(token, s.prec + !(s.type & ST_RTL_PARSED));
            if (!right) {
              token.report_error(herr, "Expected secondary expression after binary operator");
              return left_node;
            }
            left_node = new AST_Node_Binary(left_node,right,op);
            break;
          }
          if (s.type & ST_TERNARY) {
            if (s.prec < prec_min) return left_node;
            string ct(token.content.toString());
            track(ct);
            
            token = get_next_token();
            AST_Node* exptrue = parse_expression(token, 0);
            if (!exptrue) return NULL;
            if (token.type != TT_COLON) { token.report_error(herr, "Colon expected"); return NULL; }
            track(string(":"));
            
            token = get_next_token();
            AST_Node* expfalse = parse_expression(token, 0);
            if (!exptrue) return NULL;
            
            left_node = new AST_Node_Ternary(left_node,exptrue,expfalse,ct);
          }
        }
        break;
      case TT_TILDE:
      
      case TT_LEFTPARENTH:
          if (left_node->type == AT_DEFINITION or left_node->type == AT_TYPE) {
            AST_Node *params = parse_expression(token, 0);
            if (!params) {
              token.report_error(herr, "Expected secondary expression after binary operator");
              return left_node;
            }
            if (token.type != TT_RIGHTPARENTH) {
              token.report_errorf(herr, "Expected closing parenthesis here before %s");
              FATAL_RETURN(left_node);
            }
            left_node = new AST_Node_Binary(left_node,params,"(");
            break;
          }
        break;
      
      case TT_LEFTBRACKET:
      case TT_LEFTBRACE:
      case TT_COMMA:
      case TT_SEMICOLON:
      case TT_STRINGLITERAL:
      case TT_CHARLITERAL:
      case TT_DECLITERAL:
      case TT_HEXLITERAL:
      case TT_OCTLITERAL:
      
      case TT_ELLIPSIS:
      
      case TT_RIGHTPARENTH: case TT_RIGHTBRACKET: case TT_RIGHTBRACE: return left_node;
      
      case TT_TEMPLATE: case TT_NAMESPACE: case TT_ENDOFCODE: case TT_TYPEDEF:
      case TT_USING: case TT_PUBLIC: case TT_PRIVATE: case TT_PROTECTED:
      
      case TT_ASM: case TT_OPERATORKW: case TT_SIZEOF: case TT_DECLTYPE:
      return left_node;
      
      case TTM_CONCAT: case TTM_TOSTRING: case TT_INVALID: default: return left_node;
    }
    return parse_binary_or_unary_post(token,left_node,prec_min);
  }
  
  token_t AST::get_next_token() {
    return search_scope? lex->get_token_in_scope(search_scope,herr) : lex->get_token(herr);
  }
  
  
  //===========================================================================================================================
  //=: Public API :============================================================================================================
  //===========================================================================================================================
  
  int AST::parse_expression(lexer *ulex, error_handler *uherr) {
    lex = ulex, herr = uherr;
    token_t token = lex->get_token();
    if ((root = parse_expression(token, 0)))
      return 0;
    return 1;
  }
  
  int AST::parse_expression(lexer *ulex, token_t &token, error_handler *uherr) {
    lex = ulex, herr = uherr;
    token = lex->get_token();
    if ((root = parse_expression(token, 0)))
      return 0;
    return 1;
  }
  
  int AST::parse_expression(token_t &token, lexer *ulex, error_handler *uherr) {
    lex = ulex, herr = uherr;
    if ((root = parse_expression(token, 0)))
      return 0;
    return 1;
  }
  
  int AST::parse_expression(token_t &token, lexer *ulex, definition_scope *scope, error_handler *uherr) {
    search_scope = scope;
    lex = ulex, herr = uherr;
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
      dec_literal:
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
    if (type == AT_OCTLITERAL) {
      if (content.length() == 1)
        goto dec_literal; // A single octal digit is no different from a decimal digit
      return value(strtol(content.c_str(),NULL,8));
    }
    if (type == AT_HEXLITERAL) {
      if (content.length() == 1)
        goto dec_literal; // A single octal digit is no different from a decimal digit
      return value(strtol(content.c_str(),NULL,16));
    }
    return value();
  }
  value AST::AST_Node_Definition::eval() {
    if (def and (def->flags & DEF_VALUED))
      return ((definition_valued*)def)->value_of;
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
  value AST::AST_Node_Type::eval() {
    return 0L;
  }
  value AST::AST_Node_sizeof::eval() {
    return 0L; // TODO: right->coerce()->calculate_size();
  }
  value AST::AST_Node_Cast::eval() {
    if (cast_type.def == builtin_type__int)
      if (cast_type.flags & builtin_flag__long)
        if (cast_type.flags & builtin_flag__unsigned)
          return value((long)right->eval());
        else
          return value((long)(int)right->eval());
      else if (cast_type.flags & builtin_flag__short)
        if (cast_type.flags & builtin_flag__unsigned)
          return value((long)right->eval());
        else
          return value((long)(short)(long)right->eval());
      else
        if (cast_type.flags & builtin_flag__unsigned)
          return value((long)(unsigned int)(long)right->eval());
        else
          return value((long)(int)(long)right->eval());
    else if (cast_type.def == builtin_type__float)
      return value((double)(float)(double)right->eval());
    else if (cast_type.def == builtin_type__double)
      return value((double)right->eval());
    else return value();
  }
  
  
  //===========================================================================================================================
  //=: Coercers :==============================================================================================================
  //===========================================================================================================================
  
  definition* AST::AST_Node_sizeof::coerce() {
    return builtin_type__int; // FIXME: Replace with unsigned long
  }
  
  definition* AST::AST_Node_Cast::coerce() {
    return cast_type.def; // FIXME: Replace with cast_type (fulltype)
  }
  
  //===========================================================================================================================
  //=: Constructors :==========================================================================================================
  //===========================================================================================================================
  
  static string str_sizeof("sizeof",6);
  static string str_cast("cast",4);
  
  AST::AST_Node::AST_Node(): parent(NULL) {}
  AST::AST_Node::AST_Node(string ct): parent(NULL), content(ct) {}
  AST::AST_Node_Definition::AST_Node_Definition(definition* d): def(d) {}
  AST::AST_Node_Type::AST_Node_Type(full_type &ft) { dec_type.swap(ft); }
  AST::AST_Node_Unary::AST_Node_Unary(AST_Node* r): right(r) {}
  AST::AST_Node_Unary::AST_Node_Unary(AST_Node* r, string ct): AST_Node(ct), right(r) {}
  AST::AST_Node_sizeof::AST_Node_sizeof(AST_Node* param): AST_Node_Unary(param,str_sizeof) {}
  AST::AST_Node_Cast::AST_Node_Cast(AST_Node* param): AST_Node_Unary(param,str_cast) {}
  AST::AST_Node_Binary::AST_Node_Binary(AST_Node* l, AST_Node* r): left(l), right(r) { type = AT_BINARYOP; }
  AST::AST_Node_Binary::AST_Node_Binary(AST_Node* l, AST_Node* r, string op): AST_Node(op), left(l), right(r) { type = AT_BINARYOP; }
  AST::AST_Node_Ternary::AST_Node_Ternary(AST_Node *expression, AST_Node *exp_true, AST_Node *exp_false): exp(expression), left(exp_true), right(exp_false) { type = AT_TERNARYOP; }
  AST::AST_Node_Ternary::AST_Node_Ternary(AST_Node *expression, AST_Node *exp_true, AST_Node *exp_false, string ct): AST_Node(ct), exp(expression), left(exp_true), right(exp_false) { type = AT_TERNARYOP; }
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

  
  AST::AST(): root(NULL), current(NULL), search_scope(NULL) {}
    
  AST::~AST() {
    delete root;
  }
}
