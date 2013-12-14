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

#ifndef __APPLE__
#include <malloc.h>
#endif

#include <cstring>
#include <cstdio>
#include <map>

using namespace std;
using namespace jdip;

#ifdef DEBUG_MODE
#define track(ct) expression += ct + " "
#else
#define track(ct)
#endif

#include "AST_operator.h"

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
      case TT_DECFLAG: case TT_CLASS: case TT_STRUCT: case TT_ENUM: case TT_UNION: case TT_EXTERN: {
          full_type ft = read_type(lex, token, search_scope, NULL, herr); // Read the full set of declarators
          track(ft.toString());
          myroot = new AST_Node_Type(ft);
          token_basics(
              void(),
              myroot->filename = (const char*)token.file,
              myroot->linenum = token.linenum,
              myroot->pos = token.pos
            );
          handled_basics = read_next = true;
        } break;
      
      case TT_DECLARATOR: case TT_DEFINITION:
          at = AT_DEFINITION;
          myroot = new AST_Node_Definition(token.def, token.content.toString());
          track(myroot->content);
        break;
      
      case TT_IDENTIFIER: {
          if (search_scope) {
            string n(token.content.toString());
            definition *def = search_scope->look_up(n);
            if (def) {
              myroot = new AST_Node_Definition(def, token.content.toString());
              at = AT_DEFINITION;
            }
            else
              myroot = new AST_Node(token.content.toString(), at = AT_IDENTIFIER);
          }
          else
            myroot = new AST_Node(token.content.toString(), at = AT_IDENTIFIER);
          
          track(myroot->content);
        } break;
      
      case TT_OPERATORKW:
        token.report_error(herr,"Please refer to operators in their binary format; explicit use of operator functions not presently supported.");
        break;
      
      case TT_TYPENAME:
        //XXX: Is it safe to just ignore a typename directive in this system?
        token = get_next_token();
        return parse_expression(token, precedence::scope);
      case TT_TEMPLATE:
        token = get_next_token();
        if (token.type != TT_DEFINITION and token.type != TT_IDENTIFIER) {
          token.report_errorf(herr, "Expected identifier to treat as template before %s");
          return NULL;
        }
        return parse_expression(token, precedence::scope);
      
      case TT_OPERATOR: case TT_TILDE: {
        ct = token.content.toString();
        symbol& op = symbols[ct];
        if (not(op.type & ST_UNARY_PRE)) {
          token.report_error(herr,"Operator cannot be used as unary prefix");
          return NULL;
        }
        track(ct);
        token = get_next_token();
        myroot = new AST_Node_Unary(parse_expression(token, op.prec_unary_pre), ct, true);
        read_next = true;
      } break;
      
      case TT_GREATERTHAN: case TT_LESSTHAN: case TT_COLON:
        token.report_error(herr, tt_greater_is_op? "Expected expression here before greater-than operator" : "Expected expression here before closing triangle bracket");
        return NULL;
      
      case TT_SCOPE:
          cerr << "Unimplemented: '::'" << endl;
        return NULL;
      
      case TT_LEFTPARENTH:
        track(string("("));
        token = get_next_token();
        myroot = parse_expression(token, 0);
        if (myroot == NULL) return NULL;
        if (token.type != TT_RIGHTPARENTH) {
          token.report_errorf(herr, "Expected closing parenthesis here before %s");
          delete myroot;
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
        else if (myroot->type == AT_DEFINITION && (((AST_Node_Definition*)myroot)->def->flags & DEF_TYPENAME)) {
          AST_Node_Definition *ad = (AST_Node_Definition*)myroot;
          token = get_next_token(); read_next = true;
          AST_Node_Cast *nr = new AST_Node_Cast(parse_expression(token, symbols["(cast)"].prec_unary_pre));
          nr->cast_type.def = ad->def; nr->content = nr->cast_type.toString();
          delete myroot; myroot = nr;
        }
          
        handled_basics = true;
        break;
      
      case TT_LEFTBRACKET:
      case TT_LEFTBRACE: {
        AST_Node_Array* array = new AST_Node_Array();
        token = get_next_token();
        while (token.type != TT_RIGHTBRACE and token.type != TT_SEMICOLON and token.type != TT_ENDOFCODE) {
          AST_Node* n = parse_expression(token, precedence::comma + 1);
          if (!n) {
            token.report_error(herr, "Expected expression for array element");
            FATAL_RETURN(array);
          }
          array->elements.push_back(n);
          if (token.type != TT_COMMA) {
            if (token.type == TT_RIGHTBRACE) break;
            token.report_errorf(herr, "Expected comma to separate array elements before %s");
            FATAL_RETURN(array); break;
          }
          else token = get_next_token();
        }
        at = AT_ARRAY;
        myroot = array;
      }
      break;
      
      case TT_NEW: {
        token = get_next_token();
        AST_Node_new* ann = new AST_Node_new();
        if (token.type == TT_LEFTPARENTH) {
          track(string("("));
          ann->position = parse_expression(token, 0);
          if (ann->position == NULL) return NULL;
          if (token.type != TT_RIGHTPARENTH) {
            token.report_errorf(herr, "Expected closing parenthesis for placement new here before %s");
            delete ann; return NULL;
          }
          track(string(")"));
          token = get_next_token();
        }
        ann->alloc_type = read_type(lex, token, search_scope, NULL, herr);
        
        bool stillgoing = true;
        while (token.type == TT_OPERATOR) {
          if (token.content.len != 1 or *token.content.str != '*') {
            stillgoing = false;
            break;
          }
          else
            ann->alloc_type.refs.push(ref_stack::RT_POINTERTO);
        }
        
        if (stillgoing and token.type == TT_LEFTBRACKET) {
          track(string("["));
          token = get_next_token();
          ann->bound = parse_expression(token, 0);
          if (ann->bound == NULL) return NULL;
          if (token.type != TT_RIGHTBRACKET) {
            token.report_errorf(herr, "Expected closing parenthesis for placement new here before %s");
            delete ann; return NULL;
          }
          track(string("]"));
          token = get_next_token();
        }
        
        myroot = ann;
      } break;
      
      case TT_DELETE: {
        token = get_next_token();
        bool is_array = token.type == TT_LEFTBRACKET;
        if (is_array) {
          if ((token = get_next_token()).type != TT_RIGHTBRACKET) {
            token.report_errorf(herr, "Brackets to operator delete[] should be empty; expected right bracket before %s");
            return NULL;
          }
        }
        myroot = new AST_Node_delete(parse_expression(token, precedence::unary_pre), is_array);
      } break;
      
      case TT_COMMA:
      case TT_SEMICOLON:
        token.report_errorf(herr, "Expected expression before %s");
        return NULL;
      
      case TT_STRINGLITERAL:
      case TT_CHARLITERAL: myroot = new AST_Node(token.content.toString(), at = AT_CHRLITERAL);
                           track(myroot->content); break;
      
      case TT_DECLITERAL: myroot = new AST_Node(token.content.toString(), at = AT_DECLITERAL);
                          track(myroot->content); break;
      case TT_HEXLITERAL: myroot = new AST_Node(token.content.toString(), at = AT_HEXLITERAL);
                          track(myroot->content); break;
      case TT_OCTLITERAL: myroot = new AST_Node(token.content.toString(), at = AT_OCTLITERAL);
                          track(myroot->content); break;
      
      case TT_DECLTYPE:
          cerr << "Unimplemented: `decltype'." << endl;
        return NULL;
      
      {
          bool not_result;
          if (true) {
            case TT_ISEMPTY: not_result = true; track(string("isempty"));
          } else {
            case TT_SIZEOF: not_result = false; track(string("sizeof"));
          }
          if (true)
          token = get_next_token();
          if (token.type == TT_LEFTPARENTH) {
              token = get_next_token(); track(string("(")); 
              full_type ft = read_fulltype(lex, token, search_scope, NULL, herr);
              myroot = new AST_Node_sizeof(new AST_Node_Type(ft), not_result);
              if (token.type != TT_RIGHTPARENTH)
                token.report_errorf(herr, "Expected closing parenthesis to sizeof before %s");
              else { track(string(")")); }
              token = get_next_token();
          }
          else
            myroot = new AST_Node_sizeof(parse_expression(token,precedence::unary_pre), not_result);
          at = AT_UNARY_PREFIX;
          read_next = true;
      } break;
      
      case TT_ELLIPSIS:
      case TT_RIGHTPARENTH: case TT_RIGHTBRACKET: case TT_RIGHTBRACE:
        // Overflow; same error.
      case TT_NAMESPACE: case TT_ENDOFCODE: case TT_TYPEDEF: case TT_ASM:
      case TT_USING: case TT_PUBLIC: case TT_PRIVATE: case TT_PROTECTED: 
      #include <User/token_cases.h>
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
        return left_node;
      
      case TT_TYPENAME:
        token.report_error(herr, "Unimplemented: typename.");
        delete left_node;
        return NULL;
      
      case TT_COLON:
        return left_node;
      
      case TT_GREATERTHAN:
        if (!tt_greater_is_op)
          return left_node; 
        goto case_TT_OPERATOR;
        
      case TT_SCOPE: {
        track(string("::"));
        token = get_next_token();
        AST_Node *right = parse_expression(token, precedence::scope + 1);
        if (!right) {
          token.report_error(herr, "Expected qualified-id for scope access");
          return left_node;
        }
        left_node = new AST_Node_Scope(left_node,right,"::");
        break;
      }
      
      case TT_LESSTHAN:
        if (left_node->type == AT_SCOPE || left_node->type == AT_DEFINITION)
        {
          if (precedence::scope < prec_min)
            return left_node;
          definition *def = left_node->coerce().def;
          if (def && (def->flags & DEF_TEMPLATE))
          {
            track(string("<"));
            AST_Node_TempInst *ti = new AST_Node_TempInst((definition_template*)def);
            token = get_next_token();
            
            bool gtio = tt_greater_is_op;
            tt_greater_is_op = false;
            for (;;) {
              AST_Node *p = parse_expression(token, precedence::comma + 1);
              if (p == NULL) break;
              ti->params.push_back(p);
              if (token.type == TT_GREATERTHAN)
              { token = get_next_token(); break; }
              if (token.type == TT_COMMA)
              { token = get_next_token(); continue; }
              token.report_errorf(herr, "Expected closing triangle bracket before %s");
              break;
            }
            track(string(">"));
            tt_greater_is_op = gtio;
            delete left_node;
            left_node = ti;
            break;
          }
        }
          // Continue to next case
          
      case TT_OPERATOR: case_TT_OPERATOR: {
          string op(token.content.toString());
          map<string,symbol>::iterator b = symbols.find(op);
          if (b == symbols.end()) {
            token.report_error(herr, "Operator `" + token.content.toString() + "' not defined");
            delete left_node; return NULL;
          }
          if (left_node->type == AT_DEFINITION) {
            AST_Node_Definition *ad = (AST_Node_Definition*)left_node;
            if (ad->def->flags & DEF_TYPENAME) {
              full_type cme(ad->def);
              left_node = new AST_Node_Type(cme);
              delete ad;
            }
          }
          if (left_node->type == AT_TYPE) {
            if (token.content.len == 1) {
              if (*token.content.str == '*') {
                ((AST_Node_Type*)left_node)->dec_type.refs.push(ref_stack::RT_POINTERTO);
                token = get_next_token();
                break;
              }
              if (*token.content.str == '&') {
                ((AST_Node_Type*)left_node)->dec_type.refs.push(ref_stack::RT_REFERENCE);
                token = get_next_token();
                break;
              }
            }
            token.report_error(herr, "Cannot operate on type `" + ((AST_Node_Type*)left_node)->dec_type.toString() + "'");
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
              token.report_error(herr, "Expected secondary expression after binary operator " + op);
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
            if (!exptrue) {
              delete left_node;
              return NULL;
            }
            if (token.type != TT_COLON) {
              token.report_error(herr, "Colon expected to separate ternary operands");
              delete left_node;
              delete exptrue;
              return NULL;
            }
            track(string(":"));
            
            token = get_next_token();
            AST_Node* expfalse = parse_expression(token, 0);
            if (!expfalse) {
              delete left_node;
              delete exptrue;
              return NULL;
            }
            
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
          if (left_node->type == AT_DEFINITION or left_node->type == AT_TYPE or left_node->type == AT_SCOPE) {
            full_type ltype = left_node->coerce();
            if (ltype.def and ltype.def->flags & DEF_TYPENAME)
            {
              AST_Node_Type *ant;
              full_type ft_stacked, *ft_annoying;
              if (left_node->type == AT_TYPE)
                ft_annoying = &(ant = (AST_Node_Type*)left_node)->dec_type;
              else
                ant = NULL, ft_annoying = &ft_stacked;
              full_type &ft = *ft_annoying;
              
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
              
              lb.reset();
              lex = &lb;
                token = get_next_token();
                if (is_cast) {
                  read_referencers(ft.refs, ft, lex, token, search_scope, NULL, herr); // Read all referencers
                  track(ft.refs.toString());
                  if (!ant) {
                   left_node = ant = new AST_Node_Type(ft);
                    token_basics(void(),
                      left_node->filename = (const char*)token.file,
                      left_node->linenum = token.linenum,
                      left_node->pos = token.pos
                    );
                  }
                }
                else {
                  #if DEBUG_MODE
                    if (token.type != TT_LEFTPARENTH)
                      cerr << "INTERNAL ERROR: Token is not a left parenthesis. This should not happen." << endl;
                  #endif
                  track(string("("));
                  token = get_next_token();
                  AST_Node_Cast* nr = new AST_Node_Cast(parse_expression(token, 0), ft);
                  if (token.type != TT_RIGHTPARENTH)
                    token.report_errorf(herr, "Expected closing parenthesis before %s");
                  else token = get_next_token();
                  nr->content = nr->cast_type.toString();
                  delete left_node;
                  left_node = nr;
                  token_basics(
                    void(),
                    left_node->filename = (const char*)token.file,
                    left_node->linenum = token.linenum,
                    left_node->pos = token.pos
                  );
                  track(string(")"));
                }
              lex = lb.fallback_lexer;
              break;
            }
            track(string("("));
            token = get_next_token();
            
            bool gtio = tt_greater_is_op; tt_greater_is_op = true;
            AST_Node *params = parse_expression(token, precedence::all);
            tt_greater_is_op = gtio;
            if (!params) {
              token.report_error(herr, "Expected secondary expression after binary operator");
              return left_node;
            }
            if (token.type != TT_RIGHTPARENTH) {
              token.report_errorf(herr, "Expected closing parenthesis here before %s");
              FATAL_RETURN(left_node);
            }
            left_node = new AST_Node_Binary(left_node,params,"");
            token = get_next_token(); // Skip that closing paren
            track(string(")"));
            break;
          }
        return left_node;
      
      case TT_COMMA: {
          if (precedence::comma < prec_min)
            return left_node;
          token = get_next_token();
          string op(","); track(op);
          AST_Node *right = parse_expression(token, precedence::comma);
          if (!right) {
            token.report_error(herr, "Expected secondary expression after comma");
            return left_node;
          }
          left_node = new AST_Node_Binary(left_node,right,op);
        } break;
      
      case TT_LEFTBRACKET: {
          if (precedence::unary_post < prec_min)
            return left_node;
          token = get_next_token();
          string op("["); track(op);
          AST_Node *indx = parse_expression(token, precedence::comma);
          if (!indx) {
            token.report_error(herr, "Expected index for array subscript");
            return left_node;
          }
          left_node = new AST_Node_Subscript(left_node, indx);
          if (token.type != TT_RIGHTBRACKET) {
            token.report_errorf(herr, "Expected closing bracket to array subscript before %s");
            return left_node;
          } track(string("]"));
          token = get_next_token();
        } break;
      
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
      
      case TT_ASM: case TT_OPERATORKW: case TT_SIZEOF: case TT_ISEMPTY: case TT_DECLTYPE:
      case TT_NEW: case TT_DELETE:
      #include <User/token_cases.h>
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
    return !((root = parse_expression(token, precedence)));
  }
  
  int AST::parse_expression(token_t &token, lexer *ulex, int precedence, error_handler *uherr) {
    lex = ulex, herr = uherr;
    return !(root = parse_expression(token, precedence));
  }
  
  int AST::parse_expression(token_t &token, lexer *ulex, definition_scope *scope, int precedence, error_handler *uherr) {
    search_scope = scope;
    lex = ulex, herr = uherr;
    return !(root = parse_expression(token, precedence));
  }
  
  void AST::remap(const remap_set& n) {
    root->remap(n);
  }
  
  //===========================================================================================================================
  //=: Evaluators :============================================================================================================
  //===========================================================================================================================
  
  value AST::eval() const {
    if (!root) return value();
    return root->eval();
  }
  
  value AST::AST_Node::eval() const {
    if (type == AT_DECLITERAL) {
      dec_literal:
      if (is_letter(content[content.length() - 1])) {
        bool is_float = false;
        char *number = (char*)alloca(content.length());
        memcpy(number, content.c_str(), content.length() * sizeof(char));
        // This block will fail if the number is all letters--but we know it isn't.
        for (char *i = number + content.length() - 1; is_letter(*i); --i) {
          if (*i == 'f' or *i == 'd' or  *i == 'F' or *i == 'D')
            is_float = true;
          *i = 0;
        }
        if (!is_float)
          for (size_t i = 0; i < content.length(); i++)
            if (content[i] == '.' or  content[i] == 'E' or content[i] == 'e')
              is_float = true;
        if (is_float)
          return value(atof(number));
        return value(atol(number));
      }
      if (content.find('.', 0) != string::npos)
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
    if (type == AT_CHRLITERAL) {
      if (content[0] == '\'') {
        if (content[1] == '\\')
        switch (content[2]) {
          case 'n': return value(long('\n'));
          case 'r': return value(long('\r'));
          case 't': return value(long('\t'));
          case '\'': return value(long('\''));
          case '\"': return value(long('\"'));
          case '\\': return value(long('\\'));
          default: return 0L;
        }
        return value(long(content[1]));
      }
      else {
        return value(content.substr(1, content.length() - 2));
      }
    }
    return value();
  }
  value AST::AST_Node_Definition::eval() const {
    if (def) {
      if (def->flags & DEF_VALUED)
        return ((definition_valued*)def)->value_of;
      if (def->flags & DEF_TEMPPARAM)
        return value(VT_DEPENDENT);
    }
    return value();
  }
  value AST::AST_Node_Ternary::eval() const {
    if (!exp) return value();
    value e = exp->eval();
    return value_boolean(e)? (left?left->eval():value()) : (right?right->eval():value());
  }
  value AST::AST_Node_Binary::eval() const {
    if (!left or !right) return value();
    symbol_iter si = symbols.find(content);
    if (si == symbols.end()) return value();
    symbol &s = si->second;
    if (!s.operate) return value();
    value l = left->eval(), r = right->eval();
    value res = s.operate(l, r);
    return res;
  }
  value AST::AST_Node_Scope::eval() const {
    full_type res = left->coerce();
    if (!res.def or !(res.def->flags & DEF_SCOPE))
      return res.def == arg_key::abstract? value(VT_DEPENDENT) : value();
    if (res.def == arg_key::abstract)
      return value(VT_DEPENDENT);
    // cout << "Evaluating :: operator: " << res.def->name << "::" << right->content << endl;
    definition* d = ((definition_scope*)res.def)->find_local(right->content);
    if (!d or not(d->flags & DEF_VALUED)) {
      if (res.def->flags & (DEF_TEMPPARAM | DEF_HYPOTHETICAL))
        return value(VT_DEPENDENT);
      #ifdef DEBUG_MODE
        cerr << "AST evaluation failure: No `" << right->content << "' found in " << res.def->kind() << " `" << res.def->name << "'" << endl;
      #endif
      return value(long(0));
    }
    return ((definition_valued*)d)->value_of;
  }
  value AST::AST_Node_Unary::eval() const {
    if (!operand) { cerr << "No operand to unary (operator" << content << ")!" << endl; return value(); }
    if (prefix) {
      if (!symbols[content].operate_unary_pre) { cerr << "No method to unary (operator" << content << ")!" << endl; return value(); }
      value b4 = operand->eval(), after = symbols[content].operate_unary_pre(b4);
      return after;
    }
    else {
      if (!symbols[content].operate_unary_post) { cerr << "No method to unary (operator" << content << ")!" << endl; return value(); }
      value b4 = operand->eval(), after = symbols[content].operate_unary_post(b4);
      return after;
    }
  }
  /*value AST::AST_Node_Group::eval() {
    return root?root->eval():value();
  }*/
  value AST::AST_Node_Parameters::eval() const {
    return value(); // We can't evaluate a function call ;_;
  }
  value AST::AST_Node_Type::eval() const {
    return 0L;
  }
  value AST::AST_Node_sizeof::eval() const {
    if (!operand)
      return 0L;
    definition *cd = operand->coerce().def;
    return cd? (long)cd->size_of() : 0;
  }
  value AST::AST_Node_Cast::eval() const {
    if (!operand || !cast_type.def) return value();
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
    cerr << "Attempt to cast to `" << cast_type.def->name << "'" << endl;
    return value();
  }
  value AST::AST_Node_Array::eval() const {
    return elements.size()? elements.front()->eval() : value(0l);
  }
  value AST::AST_Node_new::eval() const {
    return position? position->eval() : value(0l);
  }
  value AST::AST_Node_delete::eval() const {
    return value();
  }
  value AST::AST_Node_Subscript::eval() const
  {
    value iv = index->eval();
    if (iv.type != VT_INTEGER)
      return value();
    size_t ivl = (size_t)(long)iv;
      
    if (left->type == AT_ARRAY)
    {
      AST_Node_Array* la = (AST_Node_Array*)left;
      if (ivl >= la->elements.size())
        return value();
      return la->elements[ivl]->eval();
    }
    else
    {
      value av = left->eval();
      if (av.type == VT_NONE)
        return av;
      if (av.type != VT_STRING)
        return value();
      string str = (string)av;
      if (ivl > str.length())
        return value();
      if (ivl >= str.length())
        return value(0L);
      return value(long(str[ivl]));
    }
  }
  value AST::AST_Node_TempInst::eval() const {
    return value(); // Evaluating a template instantiatoin, eg, basic_string<char>. Derp. Shouldn't happen in parser code.
  }
  value AST::AST_Node_TempKeyInst::eval() const {
    return value(); // Evaluating a template instantiatoin, eg, basic_string<char>. Derp. Shouldn't happen in parser code.
  }
  
  //===========================================================================================================================
  //=: Coercers :==============================================================================================================
  //===========================================================================================================================
  
  full_type AST::coerce() const {
    return root? root->coerce() : full_type();
  }
  
  full_type AST::AST_Node::coerce() const {
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
  
  full_type AST::AST_Node_Binary::coerce() const {
    //TODO: Implement using operator() functions.
    return left->coerce();
  }
  
  full_type AST::AST_Node_Scope::coerce() const {
    full_type res = left->coerce();
    if (!res.def or !(res.def->flags & DEF_SCOPE))
      return full_type();
    if (res.def == arg_key::abstract) return arg_key::abstract;
    res.def = ((definition_scope*)res.def)->look_up(right->content);
    return res;
  }
  
  full_type AST::AST_Node_Cast::coerce() const {
    return cast_type.def; // FIXME: Replace with cast_type (fulltype)
  }
  
  full_type AST::AST_Node_Definition::coerce() const {
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
  
  full_type AST::AST_Node_Parameters::coerce() const {
    #ifdef DEBUG_MODE
      if (func->type != AT_DEFINITION or !((AST_Node_Definition*)func)->def or !(((AST_Node_Definition*)func)->def->flags & DEF_FUNCTION)) {
        cerr << "Left-hand of parameter list not a function";
        return full_type();
      }
    #endif
    vector<full_type> param_types;
    param_types.reserve(params.size());
    for (vector<AST_Node*>::const_iterator it = params.begin(); it != params.end(); ++it)
      param_types.push_back((*it)->coerce());
    //definition_function* df = (definition_function*)((AST_Node_Definition*)func)->def;
    // TODO: Overload resolution
    // FIXME: Overload resolution!
    full_type res;
    res.def = builtin_type__int;
    //res.refs.copy(df->referencers);
    //res.refs.pop();
    //res.flags = df->flags;
    return res;
  }
  
  full_type AST::AST_Node_sizeof::coerce() const {
    full_type res;
    res.def = builtin_type__long;
    res.flags = builtin_flag__unsigned;
    return res;
  }
  
  full_type AST::AST_Node_Ternary::coerce() const {
    full_type t1 = left->coerce();
    #ifdef DEBUG_MODE
      if (t1 != right->coerce()) cerr << "ERROR: Operands to ternary operator differ in type." << endl;
    #endif
    return t1;
  }
  
  full_type AST::AST_Node_Type::coerce() const {
    full_type ret; ret.copy(dec_type);
    return ret;
  }
  
  full_type AST::AST_Node_Unary::coerce() const {
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
  
  full_type AST::AST_Node_Array::coerce() const {
    if (elements.size()) {
      full_type res = elements[0]->coerce();
      res.refs.push_array(elements.size());
      return res;
    }
    full_type res(builtin_type__int);
    res.refs.push_array(0);
    return res;
  }
  
  full_type AST::AST_Node_new::coerce() const {
    full_type res = alloc_type;
    res.refs.push_array(bound? (long)bound->eval() : 0);
    return res;
  }
  
  full_type AST::AST_Node_delete::coerce() const {
    return builtin_type__void;
  }
  
  full_type AST::AST_Node_Subscript::coerce() const {
    full_type res = left->coerce();
    if (!res.refs.size()) {
      return full_type(); // FIXME: Look up operator type
    }
    res.refs.pop(); // Dereference that bitch
    return res;
  }
  
  full_type AST::AST_Node_TempInst::coerce() const {
    arg_key k(temp->params.size());
    k.mirror(temp);
    for (size_t i = 0; i < params.size(); ++i) {
      if (temp->params[i]->flags & DEF_TYPENAME) {
        full_type t = params[i]->coerce();
        if (t.def == arg_key::abstract || t.def->flags & (DEF_TEMPPARAM | DEF_HYPOTHETICAL))
          return arg_key::abstract;
        k.put_type(i, t);
      }
      else {
        value v = params[i]->eval();
        if (v.type == VT_DEPENDENT) // Bail out returning void if we have dependent parameters
          return arg_key::abstract;
        k.put_value(i, v);
      }
    }
    check_read_template_parameters(k, params.size(), temp, token_t(), def_error_handler);
    return full_type(temp->instantiate(k, def_error_handler));
  }
  full_type AST::AST_Node_TempKeyInst::coerce() const {
    if (key.is_abstract()) return arg_key::abstract;
    return full_type(temp->instantiate(key, def_error_handler));
  }
  
  //===========================================================================================================================
  //=: Constructors :==========================================================================================================
  //===========================================================================================================================
  
  static string str_sizeof("sizeof",6);
  static string str_cast("cast",4);
  
  AST::AST_Node::AST_Node(AST_TYPE tp): type(tp) {}
  AST::AST_Node::AST_Node(string ct, AST_TYPE tp): type(tp), content(ct) {}
  AST::AST_Node_Definition::AST_Node_Definition(definition* d, string ct): AST_Node(ct, AT_DEFINITION), def(d) {}
  AST::AST_Node_Scope::AST_Node_Scope(AST_Node* l, AST_Node* r, string op): AST_Node_Binary(l,r,op, AT_SCOPE) {}
  AST::AST_Node_Type::AST_Node_Type(full_type &ft): AST_Node(AT_TYPE) { dec_type.swap(ft); }
  AST::AST_Node_Unary::AST_Node_Unary(AST_TYPE tp, AST_Node* r): AST_Node(tp), operand(r) {}
  AST::AST_Node_Unary::AST_Node_Unary(AST_Node* r, string ct, bool pre, AST_TYPE tp): AST_Node(ct, tp), operand(r), prefix(pre) {}
  AST::AST_Node_Unary::AST_Node_Unary(AST_Node* r, string ct, bool pre): AST_Node(ct, pre? AT_UNARY_PREFIX : AT_UNARY_POSTFIX), operand(r), prefix(pre) {}
  AST::AST_Node_sizeof::AST_Node_sizeof(AST_Node* param, bool n): AST_Node_Unary(param, str_sizeof, AT_SIZEOF), negate(n) {}
  AST::AST_Node_Cast::AST_Node_Cast(AST_Node* param, const full_type& ft): AST_Node_Unary(param, str_cast, AT_CAST) { cast_type.copy(ft); }
  AST::AST_Node_Cast::AST_Node_Cast(AST_Node* param, full_type& ft): AST_Node_Unary(param, str_cast, AT_CAST) { cast_type.swap(ft); }
  AST::AST_Node_Cast::AST_Node_Cast(AST_Node* param): AST_Node_Unary(param, str_cast, AT_CAST) {}
  AST::AST_Node_Binary::AST_Node_Binary(AST_TYPE tp, AST_Node* l, AST_Node* r): AST_Node(tp), left(l), right(r) { type = AT_BINARYOP; }
  AST::AST_Node_Binary::AST_Node_Binary(AST_Node* l, AST_Node* r, string op, AST_TYPE tp): AST_Node(op, tp), left(l), right(r) { type = tp; }
  AST::AST_Node_Ternary::AST_Node_Ternary(AST_Node *expression, AST_Node *exp_true, AST_Node *exp_false): AST_Node(AT_TERNARYOP), exp(expression), left(exp_true), right(exp_false) { type = AT_TERNARYOP; }
  AST::AST_Node_Ternary::AST_Node_Ternary(AST_Node *expression, AST_Node *exp_true, AST_Node *exp_false, string ct): AST_Node(ct, AT_TERNARYOP), exp(expression), left(exp_true), right(exp_false) { type = AT_TERNARYOP; }
  AST::AST_Node_Parameters::AST_Node_Parameters(): AST_Node(AT_PARAMLIST), func(NULL) {}
  AST::AST_Node_new::AST_Node_new(const full_type &t, AST_Node *p, AST_Node *b): AST_Node(AT_NEW), alloc_type(t), position(p), bound(b) {}
  AST::AST_Node_new::AST_Node_new(): AST_Node(AT_NEW), alloc_type(), position(NULL), bound(NULL) {}
  AST::AST_Node_delete::AST_Node_delete(AST_Node* param, bool arr): AST_Node_Unary(param, arr? "delete" : "delete[]", AT_DELETE), array(arr) {}
  AST::AST_Node_Subscript::AST_Node_Subscript(AST_Node* l, AST_Node *ind): AST_Node(AT_SUBSCRIPT), left(l), index(ind) {}
  AST::AST_Node_Subscript::AST_Node_Subscript(): AST_Node(AT_SUBSCRIPT), left(NULL), index(NULL) {}
  AST::AST_Node_TempInst::AST_Node_TempInst(definition_template* d): AST_Node(d->name, AT_INSTANTIATE), temp(d) {}
  AST::AST_Node_TempKeyInst::AST_Node_TempKeyInst(definition_template* t, const arg_key &k): AST_Node(t->name, AT_INSTBYKEY), temp(t), key(k) {}
  AST::AST_Node_Array::AST_Node_Array(): AST_Node(AT_ARRAY) {}
  
  
  //===========================================================================================================================
  //=: Destructors :===========================================================================================================
  //===========================================================================================================================
  
  AST::AST_Node::~AST_Node() { }
  AST::AST_Node_Binary::~AST_Node_Binary() { delete left; delete right; }
  AST::AST_Node_Unary::~AST_Node_Unary() { delete operand; }
  AST::AST_Node_Ternary::~AST_Node_Ternary() { delete exp; delete left; delete right; }
  AST::AST_Node_Parameters::~AST_Node_Parameters() { for (size_t i = 0; i < params.size(); i++) delete params[i]; }
  AST::AST_Node_TempInst::~AST_Node_TempInst() { for (size_t i = 0; i < params.size(); i++) delete params[i]; }
  AST::AST_Node_TempKeyInst::~AST_Node_TempKeyInst() { }
  AST::AST_Node_Array::~AST_Node_Array() { for (vector<AST_Node*>::iterator it = elements.begin(); it != elements.end(); ++it) delete *it; }
  AST::AST_Node_Subscript::~AST_Node_Subscript() { delete left; delete index; }
  
  
  //===========================================================================================================================
  //=: ASTOperator Class :=====================================================================================================
  //===========================================================================================================================
  
  void AST::AST_Node            ::operate(ASTOperator *aop, void *param) { aop->operate            (this, param); }
  void AST::AST_Node_Definition ::operate(ASTOperator *aop, void *param) { aop->operate_Definition (this, param); }
  void AST::AST_Node_Scope      ::operate(ASTOperator *aop, void *param) { aop->operate_Scope      (this, param); }
  void AST::AST_Node_Type       ::operate(ASTOperator *aop, void *param) { aop->operate_Type       (this, param); }
  void AST::AST_Node_Unary      ::operate(ASTOperator *aop, void *param) { aop->operate_Unary      (this, param); }
  void AST::AST_Node_sizeof     ::operate(ASTOperator *aop, void *param) { aop->operate_sizeof     (this, param); }
  void AST::AST_Node_Cast       ::operate(ASTOperator *aop, void *param) { aop->operate_Cast       (this, param); }
  void AST::AST_Node_Binary     ::operate(ASTOperator *aop, void *param) { aop->operate_Binary     (this, param); }
  void AST::AST_Node_Ternary    ::operate(ASTOperator *aop, void *param) { aop->operate_Ternary    (this, param); }
  void AST::AST_Node_Parameters ::operate(ASTOperator *aop, void *param) { aop->operate_Parameters (this, param); }
  void AST::AST_Node_Array      ::operate(ASTOperator *aop, void *param) { aop->operate_Array      (this, param); }
  void AST::AST_Node_new        ::operate(ASTOperator *aop, void *param) { aop->operate_new        (this, param); }
  void AST::AST_Node_delete     ::operate(ASTOperator *aop, void *param) { aop->operate_delete     (this, param); }
  void AST::AST_Node_Subscript  ::operate(ASTOperator *aop, void *param) { aop->operate_Subscript  (this, param); }
  void AST::AST_Node_TempInst   ::operate(ASTOperator *aop, void *param) { aop->operate_TempInst   (this, param); }
  void AST::AST_Node_TempKeyInst::operate(ASTOperator *aop, void *param) { aop->operate_TempKeyInst(this, param); }
  
  void AST::AST_Node            ::operate(ConstASTOperator *aop, void *param) const { aop->operate            (this, param); }
  void AST::AST_Node_Definition ::operate(ConstASTOperator *aop, void *param) const { aop->operate_Definition (this, param); }
  void AST::AST_Node_Scope      ::operate(ConstASTOperator *aop, void *param) const { aop->operate_Scope      (this, param); }
  void AST::AST_Node_Type       ::operate(ConstASTOperator *aop, void *param) const { aop->operate_Type       (this, param); }
  void AST::AST_Node_Unary      ::operate(ConstASTOperator *aop, void *param) const { aop->operate_Unary      (this, param); }
  void AST::AST_Node_sizeof     ::operate(ConstASTOperator *aop, void *param) const { aop->operate_sizeof     (this, param); }
  void AST::AST_Node_Cast       ::operate(ConstASTOperator *aop, void *param) const { aop->operate_Cast       (this, param); }
  void AST::AST_Node_Binary     ::operate(ConstASTOperator *aop, void *param) const { aop->operate_Binary     (this, param); }
  void AST::AST_Node_Ternary    ::operate(ConstASTOperator *aop, void *param) const { aop->operate_Ternary    (this, param); }
  void AST::AST_Node_Parameters ::operate(ConstASTOperator *aop, void *param) const { aop->operate_Parameters (this, param); }
  void AST::AST_Node_Array      ::operate(ConstASTOperator *aop, void *param) const { aop->operate_Array      (this, param); }
  void AST::AST_Node_new        ::operate(ConstASTOperator *aop, void *param) const { aop->operate_new        (this, param); }
  void AST::AST_Node_delete     ::operate(ConstASTOperator *aop, void *param) const { aop->operate_delete     (this, param); }
  void AST::AST_Node_Subscript  ::operate(ConstASTOperator *aop, void *param) const { aop->operate_Subscript  (this, param); }
  void AST::AST_Node_TempInst   ::operate(ConstASTOperator *aop, void *param) const { aop->operate_TempInst   (this, param); }
  void AST::AST_Node_TempKeyInst::operate(ConstASTOperator *aop, void *param) const { aop->operate_TempKeyInst(this, param); }
  
  //===========================================================================================================================
  //=: Everything else :=======================================================================================================
  //===========================================================================================================================
  
  void AST::clear() {
    delete root;
    #ifdef DEBUG_MODE
      expression.clear();
    #endif
    root = NULL;
  }
  bool AST::empty() {
    return !root;
  }
  
  void AST::swap(AST& o) {
    AST_Node *r = o.root;
    o.root = root;
    root = r;
  }
  
  AST::AST(): root(NULL), search_scope(NULL), tt_greater_is_op(true) {}
  AST::AST(AST_Node* r): root(r), search_scope(NULL), tt_greater_is_op(true) {}
  AST::AST(AST_Node* r, definition_scope *ss): root(r), search_scope(ss), tt_greater_is_op(true) {}
  AST::AST(definition* d): root(new AST_Node_Definition(d, d->name)), search_scope(NULL), tt_greater_is_op(true) {}
  
  AST* AST::create_from_instantiation(definition_template* temp, const arg_key &key) { return new AST(new AST_Node_TempKeyInst(temp, key), NULL); }
  AST* AST::create_from_access(definition_scope* scope, string id, string scope_op) {
    return new AST(
      new AST_Node_Scope(
        new AST_Node_Definition(
          scope,
          scope->name
        ),
        new AST_Node(
          id,
          AT_IDENTIFIER
        ),
        scope_op
      ), NULL
    );
  }
    
  AST::~AST() {
    delete root;
  }
}
