/**
 * @file  AST.cpp
 * @brief Source implementing a class that handles expression evaluation and type
 *        coercion via an AST.
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

#include "AST.h"
#include <General/parse_basics.h>
#include <Storage/value_funcs.h>
#include <System/builtins.h>
#include <System/symbols.h>
#include <System/lex_buffer.h>
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
          full_type ft = read_type(lex, token, search_scope, NULL, herr); // Read the full set of declarators
          track(ft.toString());
          if (token.type == TT_LEFTPARENTH) {
            lex_buffer lb(lex);
            bool is_cast = true; // True if the contents of these parentheses are part of the cast;
            // For example, in bool(*)(), the (*) is part of the cast. In bool(10), (10) is not part of the cast.
            
            lb.push(token);
            for (int depth = 1;;) {
              token_t &tk = lb.push(get_next_token());
              if (tk.type == TT_RIGHTPARENTH) {
                  if (!--depth) { token = tk; break; }
              } else if (tk.type == TT_LEFTPARENTH) ++depth;
              else if (tk.type == TT_ENDOFCODE) break;
              else if (tk.type != TT_OPERATOR or tk.content.len != 1
                   or (*tk.content.str != '*' and *tk.content.str != '&'))
                is_cast = false;
            }
            if (token.type != TT_RIGHTPARENTH) {
              token.report_errorf(herr, "Expected closing parenthesis to cast here before %s");
              return NULL;
            }
            
            lb.reset(); lex = &lb;
            token = get_next_token();
            if (is_cast) {
              read_referencers(ft.refs, lex, token, search_scope, NULL, herr); // Read all referencers
              track(ft.refs.toString());
              myroot = new AST_Node_Type(ft);
              token_basics(
                myroot->type = AT_TYPE,
                myroot->filename = (const char*)token.file,
                myroot->linenum = token.linenum,
                myroot->pos = token.pos
              );
            }
            else {
              AST_Node_Cast* nr = new AST_Node_Cast(parse_expression(token, 0), ft);
              nr->content = nr->cast_type.toString();
              myroot = nr;
              token_basics(
                myroot->type = AT_UNARY_PREFIX,
                myroot->filename = (const char*)token.file,
                myroot->linenum = token.linenum,
                myroot->pos = token.pos
              );
            }
            lex = lb.fallback_lexer;
          }
          else {
            read_referencers(ft.refs, lex, token, search_scope, NULL, herr); // Read all referencers
            myroot = new AST_Node_Type(ft);
            token_basics(
              myroot->type = AT_TYPE,
              myroot->filename = (const char*)token.file,
              myroot->linenum = token.linenum,
              myroot->pos = token.pos
            );
          }
          if (token.type == TT_RIGHTPARENTH) // Facilitate casts
            return myroot;
          handled_basics = read_next = true;
        } break;
      
      case TT_IDENTIFIER: case TT_DEFINITION: {
          if (search_scope) {
            string n(token.content.toString());
            definition *def = search_scope->look_up(n);
            if (def) {
              myroot = new AST_Node_Definition(def);
              at = AT_DEFINITION;
            }
            else {
              myroot = new AST_Node();
              at = AT_IDENTIFIER;
            }
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
        token.report_error(herr, "Unimplemented: typename.");
        return NULL;
      
      case TT_OPERATOR: case TT_TILDE: {
        ct = string(token.content.toString());
        symbol& op = symbols[ct];
        if (not(op.type & ST_UNARY_PRE)) {
          token.report_error(herr,"Operator cannot be used as unary prefix");
          return NULL;
        }
        track(ct);
        token = get_next_token();
        myroot = new AST_Node_Unary(parse_expression(token, op.prec_unary_pre), ct, true);
        if (!myroot) return NULL;
        read_next = true;
      } break;
      
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
          AST_Node_Cast *nr = new AST_Node_Cast(parse_expression(token, symbols["(cast)"].prec_unary_pre));
          nr->cast_type.swap(ad->dec_type); nr->content = nr->cast_type.toString();
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
              myroot = new AST_Node_sizeof(parse_expression(token,precedence::max));
              if (token.type != TT_RIGHTPARENTH)
                token.report_errorf(herr, "Expected closing parenthesis to sizeof before %s");
              else { track(string(")")); }
              token = get_next_token();
          }
          else
            myroot = new AST_Node_sizeof(parse_expression(token,precedence::unary_pre));
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
      
      case TTM_CONCAT: case TTM_TOSTRING: token.report_error(herr, "Illogical token type returned!");
        return NULL;
      case TT_INVALID: default: token.report_error(herr, "Invalid token type returned!");
        return NULL;
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
        token.report_errorf(herr, "Expected operator before %s");
        return NULL;
      
      case TT_TYPENAME:
        token.report_error(herr, "Unimplemented: typename.");
        return NULL;
      
      case TT_COLON:
        return left_node;
      
      case TT_LESSTHAN: {
        if (precedence::scope <= prec_min)
          return left_node;
        full_type lt = left_node->coerce();
        if (lt.def and (lt.def->flags & DEF_TEMPLATE)) {
          definition_template::arg_key k(((definition_template*)lt.def)->params.size());
          if (read_template_parameters(k, (definition_template*)lt.def, lex, token, search_scope, NULL, herr))
            return NULL;
          delete left_node;
          definition *d = ((definition_template*)lt.def)->instantiate(k);
          if (d->flags & DEF_TYPENAME) {
            lt.def = d;
            left_node = new AST_Node_Type(lt);
          }
          else
            left_node = new AST_Node_Definition(d);
          token = get_next_token();
          break;
        }
      } goto case_TT_OPERATOR;
      
      case TT_GREATERTHAN:
        if (!tt_greater_is_op)
          return left_node; 
        goto case_TT_OPERATOR;
        
      case TT_SCOPE: {
        token = get_next_token();
        AST_Node *right = parse_expression(token, precedence::scope + 1);
        if (!right) {
          token.report_error(herr, "Expected secondary expression after binary operator");
          return left_node;
        }
        left_node = new AST_Node_Scope(left_node,right,"::");
        break;
      }
      case TT_OPERATOR: case_TT_OPERATOR: {
          string op(token.content.toString());
          map<string,symbol>::iterator b = symbols.find(op);
          if (b == symbols.end()) {
            token.report_error(herr, "Operator `" + token.content.toString() + "' not defined");
            return NULL;
          }
          symbol &s = b->second;
          if (s.type & ST_BINARY) {
            if (s.prec_binary < prec_min)
              return left_node;
            token = get_next_token();
            track(op);
            AST_Node *right = parse_expression(token, s.prec_binary + !(s.type & ST_RTL_PARSED));
            if (!right) {
              token.report_error(herr, "Expected secondary expression after binary operator");
              return left_node;
            }
            left_node = new AST_Node_Binary(left_node,right,op);
            break;
          }
          if (s.type & ST_TERNARY) {
            if (s.prec_binary < prec_min)
              return left_node;
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
            break;
          }
          if (s.type & ST_UNARY_POST) {
            if (s.prec_unary_post < prec_min)
              return left_node;
            left_node = new AST_Node_Unary(left_node, op, false); 
            token = get_next_token();
          }
        }
        break;
      case TT_TILDE:
      
      case TT_LEFTPARENTH:
          if (left_node->type == AT_DEFINITION or left_node->type == AT_TYPE) {
            token = get_next_token();
            AST_Node *params = parse_expression(token, precedence::all);
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
      
      case TT_COMMA: {
          if (precedence::comma < prec_min)
            return left_node;
          token = get_next_token();
          string op(","); track(op);
          AST_Node *right = parse_expression(token, precedence::comma);
          if (!right) {
            token.report_error(herr, "Expected secondary expression after binary operator");
            return left_node;
          }
          left_node = new AST_Node_Binary(left_node,right,op);
        } break;
      
      case TT_LEFTBRACKET:
      case TT_LEFTBRACE:
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
  
  int AST::parse_expression(lexer *ulex, token_t &token, int precedence, error_handler *uherr) {
    lex = ulex, herr = uherr;
    token = get_next_token();
    if ((root = parse_expression(token, precedence)))
      return 0;
    return 1;
  }
  
  int AST::parse_expression(token_t &token, lexer *ulex, int precedence, error_handler *uherr) {
    lex = ulex, herr = uherr;
    if ((root = parse_expression(token, precedence)))
      return 0;
    return 1;
  }
  
  int AST::parse_expression(token_t &token, lexer *ulex, definition_scope *scope, int precedence, error_handler *uherr) {
    search_scope = scope;
    lex = ulex, herr = uherr;
    root = parse_expression(token, precedence);
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
        if (content[content.length()-1] == 'f' or content[content.length()-1] == 'd'
        or  content[content.length()-1] == 'F' or content[content.length()-1] == 'D')
          is_float = true;
        content.erase(content.length()-1);
      }
      if (!is_float)
        for (size_t i = 0; i < content.length(); i++)
          if (content[i] == '.' or  content[i] == 'E' or content[i] == 'e')
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
  value AST::AST_Node_Scope::eval() {
    full_type res = left->coerce();
    if (!res.def or !(res.def->flags & DEF_SCOPE))
      return value();
    definition* d = ((definition_scope*)res.def)->look_up(right->content);
    if (!d or not(d->flags & DEF_VALUED))
      return value();
    return ((definition_valued*)d)->value_of;
  }
  value AST::AST_Node_Unary::eval() {
    if (!operand) { cout << "No operand to unary (operator" << content << ")!" << endl; return value(); }
    if (prefix) {
      if (!symbols[content].operate_unary_pre) { cout << "No method to unary (operator" << content << ")!" << endl; return value(); }
      value b4 = operand->eval(), after = symbols[content].operate_unary_pre(b4);
      return after;
    }
    else {
      if (!symbols[content].operate_unary_post) { cout << "No method to unary (operator" << content << ")!" << endl; return value(); }
      value b4 = operand->eval(), after = symbols[content].operate_unary_post(b4);
      return after;
    }
  }
  /*value AST::AST_Node_Group::eval() {
    return root?root->eval():value();
  }*/
  value AST::AST_Node_Parameters::eval() {
    return value(); // We can't evaluate a function call ;_;
  }
  value AST::AST_Node_Type::eval() {
    return 0L;
  }
  value AST::AST_Node_sizeof::eval() {
    return (long)operand->coerce().def->size_of();
  }
  value AST::AST_Node_Cast::eval() {
    if (cast_type.def == builtin_type__int)
      if (cast_type.flags & builtin_flag__long)
        if (cast_type.flags & builtin_flag__unsigned)
          return value((long)operand->eval());
        else
          return value((long)(int)operand->eval());
      else if (cast_type.flags & builtin_flag__short)
        if (cast_type.flags & builtin_flag__unsigned)
          return value((long)operand->eval());
        else
          return value((long)(short)(long)operand->eval());
      else
        if (cast_type.flags & builtin_flag__unsigned)
          return value((long)(unsigned int)(long)operand->eval());
        else
          return value((long)(int)(long)operand->eval());
    else if (cast_type.def == builtin_type__float)
      return value((double)(float)(double)operand->eval());
    else if (cast_type.def == builtin_type__double)
      return value((double)operand->eval());
    else if (cast_type.def == builtin_type__bool)
      return value((long)(bool)operand->eval());
    cout << "WELL, FUCK." << endl;
    return value();
  }
  
  
  //===========================================================================================================================
  //=: Coercers :==============================================================================================================
  //===========================================================================================================================
  
  full_type AST::coerce() {
    return root? root->coerce() : full_type();
  }
  
  full_type AST::AST_Node::coerce() {
    full_type res;
    res.def = builtin_type__int;
    res.flags = 0;
    for (size_t i = content.length(); i and is_letter(content[i]); --i)
      if (content[i] == 'l' or content[i] == 'L') res.flags |= builtin_flag__long;
      else if (content[i] == 'u' or content[i] == 'U') res.flags |= builtin_flag__unsigned;
    if (type == AT_DECLITERAL)
      for (size_t i = 0; i < content.length(); ++i)
        if (content[i] == '.' or content[i] == 'e' or content[i] == 'E') {
          res.def = builtin_type__double;
          while (++i < content.length()) if (content[i] == 'f' or content[i] == 'F')
            res.def = builtin_type__float;
          break;
        }
    return res;
  }
  
  full_type AST::AST_Node_Binary::coerce() {
    //TODO: Implement using operator() functions.
    return left->coerce();
  }
  
  full_type AST::AST_Node_Scope::coerce() {
    full_type res = left->coerce();
    if (!res.def or !(res.def->flags & DEF_SCOPE))
      return full_type();
    res.def = ((definition_scope*)res.def)->look_up(right->content);
    return res;
  }
  
  full_type AST::AST_Node_Cast::coerce() {
    return cast_type.def; // FIXME: Replace with cast_type (fulltype)
  }
  
  full_type AST::AST_Node_Definition::coerce() {
    full_type res;
    if (def->flags & DEF_TYPED) {
      res.def = ((definition_typed*)def)->type;
      res.refs.copy(((definition_typed*)def)->referencers);
      res.flags = ((definition_typed*)def)->modifiers;
    }
    else
      res.def = def;
    return res;
  }
  
  full_type AST::AST_Node_Parameters::coerce() {
    #ifdef DEBUG_MODE
      if (func->type != AT_DEFINITION or !((AST_Node_Definition*)func)->def or !(((AST_Node_Definition*)func)->def->flags & DEF_FUNCTION)) {
        cerr << "Left-hand of parameter list not a function";
        return full_type();
      }
    #endif
    vector<full_type> param_types;
    param_types.reserve(params.size());
    for (vector<AST_Node*>::iterator it = params.begin(); it != params.end(); ++it)
      param_types.push_back((*it)->coerce());
    definition_function* df = (definition_function*)((AST_Node_Definition*)func)->def;
    // TODO: Overload resolution
    full_type res;
    res.def = df->type;
    res.refs.copy(df->referencers);
    res.refs.pop();
    res.flags = df->flags;
    return res;
  }
  
  full_type AST::AST_Node_sizeof::coerce() {
    full_type res;
    res.def = builtin_type__long;
    res.flags = builtin_flag__unsigned;
    return res;
  }
  
  full_type AST::AST_Node_Ternary::coerce() {
    full_type t1 = left->coerce();
    #ifdef DEBUG_MODE
      if (t1 != right->coerce()) cerr << "ERROR: Operands to ternary operator differ in type." << endl;
    #endif
    return t1;
  }
  
  full_type AST::AST_Node_Type::coerce() {
    full_type ret; ret.copy(dec_type);
    return ret;
  }
  
  full_type AST::AST_Node_Unary::coerce() {
    switch (content[0]) {
      case '+':
      case '-':
      case '~': return operand->coerce();
      case '*': { full_type res = operand->coerce(); res.refs.pop(); return res; }
      case '&': { full_type res = operand->coerce(); res.refs.push(ref_stack::RT_POINTERTO); return res; }
      case '!': return builtin_type__bool;
      default:
        #ifdef DEBUG_MODE
          cerr << "ERROR: Unknown coercion pattern for ternary operator `" << content << "'" << endl;
        #endif
        return operand->coerce();
    }
  }
  
  //===========================================================================================================================
  //=: Constructors :==========================================================================================================
  //===========================================================================================================================
  
  static string str_sizeof("sizeof",6);
  static string str_cast("cast",6);
  
  AST::AST_Node::AST_Node(): parent(NULL) {}
  AST::AST_Node::AST_Node(string ct): parent(NULL), content(ct) {}
  AST::AST_Node_Definition::AST_Node_Definition(definition* d): def(d) {}
  AST::AST_Node_Scope::AST_Node_Scope(AST_Node* l, AST_Node* r, string op): AST_Node_Binary(l,r,op) {}
  AST::AST_Node_Type::AST_Node_Type(full_type &ft) { dec_type.swap(ft); }
  AST::AST_Node_Unary::AST_Node_Unary(AST_Node* r): operand(r) {}
  AST::AST_Node_Unary::AST_Node_Unary(AST_Node* r, string ct, bool pre): AST_Node(ct), operand(r), prefix(pre) {}
  AST::AST_Node_sizeof::AST_Node_sizeof(AST_Node* param): AST_Node_Unary(param,str_sizeof, true) {}
  AST::AST_Node_Cast::AST_Node_Cast(AST_Node* param, const full_type& ft): AST_Node_Unary(param, str_cast, true) { cast_type.copy(ft); }
  AST::AST_Node_Cast::AST_Node_Cast(AST_Node* param, full_type& ft): AST_Node_Unary(param, str_cast, true) { cast_type.swap(ft); }
  AST::AST_Node_Cast::AST_Node_Cast(AST_Node* param): AST_Node_Unary(param, str_cast, true) {}
  AST::AST_Node_Binary::AST_Node_Binary(AST_Node* l, AST_Node* r): left(l), right(r) { type = AT_BINARYOP; }
  AST::AST_Node_Binary::AST_Node_Binary(AST_Node* l, AST_Node* r, string op): AST_Node(op), left(l), right(r) { type = AT_BINARYOP; }
  AST::AST_Node_Ternary::AST_Node_Ternary(AST_Node *expression, AST_Node *exp_true, AST_Node *exp_false): exp(expression), left(exp_true), right(exp_false) { type = AT_TERNARYOP; }
  AST::AST_Node_Ternary::AST_Node_Ternary(AST_Node *expression, AST_Node *exp_true, AST_Node *exp_false, string ct): AST_Node(ct), exp(expression), left(exp_true), right(exp_false) { type = AT_TERNARYOP; }
  AST::AST_Node_Parameters::AST_Node_Parameters(): func(NULL) {}
  
  
  //===========================================================================================================================
  //=: Destructors :===========================================================================================================
  //===========================================================================================================================
  
  AST::AST_Node::~AST_Node() { }
  AST::AST_Node_Binary::~AST_Node_Binary() { delete left; delete right; }
  AST::AST_Node_Unary::~AST_Node_Unary() { delete operand; }
  AST::AST_Node_Ternary::~AST_Node_Ternary() { delete exp; delete left; delete right; }
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

  
  AST::AST(): root(NULL), current(NULL), search_scope(NULL), tt_greater_is_op(true) {}
    
  AST::~AST() {
    delete root;
  }
}
