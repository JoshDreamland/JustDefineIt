/**
 * @file read_type.cpp
 * @brief Source implementing a utility funciton to read in a type.
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

#include <Parser/bodies.h>
#include <API/context.h>
#include <API/AST.h>
#include <API/error_reporting.h>
#include <General/parse_basics.h>
#include <General/debug_macros.h>
#include <Parser/parse_context.h>
#include <System/builtins.h>
#include <cstdio>
using namespace jdip;
using namespace jdi;

full_type jdip::read_type(lexer *lex, token_t &token, definition_scope *scope, error_handler *herr)
{
  definition* inferred_type = NULL;
  definition* overridable_type = NULL;
  long int rflags = 0; // Return flags.
  long int swif = 0; // Swap-in flags: or'd in when a flag is determined not to be the main type.
  definition *rdef = NULL;
  ref_stack rrefs;
  
  if (token.type != TT_DECLARATOR) {
    if (token.type != TT_DECFLAG) {
      if (token.type != TT_DECLARATOR) {
        token.report_error(herr,"Type name expected here");
        return full_type();
      }
    }
    else {
      typeflag *const tf = ((typeflag*)token.extra.def);
      if (tf->usage & UF_PRIMITIVE)
        overridable_type = tf->def,
        swif = tf->flagbit;
      else {
        if (tf->usage & UF_STANDALONE)
          inferred_type = tf->def;
        rflags = tf->flagbit;
      }
    }
  }
  else
    rdef = token.extra.def;
  
  // Read any additional type info
  token = lex->get_token();
  while (token.type == TT_DECLARATOR or token.type == TT_DECFLAG)
  {
    if (token.type == TT_DECLARATOR) {
      if (rdef) {
        token.report_error(herr,"Two types named in declaration");
        return full_type();
      }
      rdef = token.extra.def;
      rflags |= swif;
    }
    else {
      typeflag *const tf = ((typeflag*)token.extra.def);
      if (tf->usage & UF_PRIMITIVE)
        overridable_type = tf->def,
        rflags |= swif,
        swif = tf->flagbit;
      else if (tf->usage & UF_STANDALONE_FLAG)
        inferred_type = tf->def,
        rflags |= tf->flagbit;
    }
    token = lex->get_token();
  }
  
  if (rdef == NULL)
    rdef = overridable_type;
  if (rdef == NULL)
    rdef = inferred_type;
  if (rdef)
    jdip::read_referencers(rrefs, lex, token, scope, herr);
  return full_type(rdef, rrefs, rflags);
}

#define render_ast(x)
#ifdef DEBUG_MODE
  #ifdef RENDER_ASTS
    #include <sys/stat.h>
    #undef render_ast
    static unsigned ast_rn = 0;
    static void render_ast(AST& ast) {
      if (!ast_rn) {
        mkdir("/home/josh/Desktop/AST_Renders",0777);
        mkdir("/home/josh/Desktop/AST_Renders/ArrayBounds",0777);
      }
      char fn[64]; sprintf(fn,"/home/josh/Desktop/AST_Renders/ArrayBounds/ast_%08u.svg",ast_rn++);
      ast.writeSVG(fn);
    }
  #endif
#endif

int jdip::read_referencers(ref_stack &refs, lexer *lex, token_t &token, definition_scope *scope, error_handler *herr)
{
  ref_stack append;
  ref_stack postfix;
  bool rhs = false;
  for (;;)
  {
    switch (token.type)
    {
      case TT_LEFTBRACKET: { // Array bound indicator
        AST ast;
        rhs = true;
        if (ast.parse_expression(lex,token,herr))
          return 1; // This error has already been reported, just return empty.
        if (token.type != TT_RIGHTBRACKET) {
          token.report_error(herr,"Expected closing square bracket here before %s");
          return 1;
        }
        render_ast(ast);
        value as = ast.eval();
        size_t boundsize = (as.type == VT_INTEGER)? as.val.i : ref_stack::node_array::nbound;
        postfix.push_array(boundsize);
      } break;
      case TT_LEFTPARENTH: { // Either a function or a grouping
        token = lex->get_token(herr);
        if (!rhs) { // If we're still on the left-hand side
          rhs = true;
          read_referencers(append, lex, token, scope, herr);
          if (token.type != TT_RIGHTPARENTH) {
            token.report_error(herr, "Expected right parenthesis after nested referencers");
          }
        }
        else {
          ref_stack::parameter_ct params;
          while (token.type != TT_RIGHTPARENTH)
          {
            full_type a = read_type(lex,token,scope,herr);
            params.throw_on(a);
            if (token.type != TT_COMMA) {
              if (token.type == TT_RIGHTPARENTH) break;
              token.report_error(herr,"Expected comma or closing parenthesis to function parameters");
              return 1;
            }
            token = lex->get_token();
          }
          postfix.push_func(params);
          if (token.type != TT_RIGHTPARENTH) {
            token.report_error(herr,"Expected closing parenthesis to function parameters");
            return 1;
          }
        }
      } break;
      case TT_IDENTIFIER: // The name associated with this type, be it the name of a parameter or of a declaration or what have you.
        refs.name = string((const char*)token.extra.content.str, token.extra.content.len);
        rhs = true; // Officially on the right hand side
      break;
      
      
      case TT_OPERATOR: // Could be an asterisk or ampersand
        if ((token.extra.content.str[0] == '&' or token.extra.content.str[0] == '*') and token.extra.content.len == 1) {
          refs.push(token.extra.content.str[0] == '&'? ref_stack::RT_REFERENCE : ref_stack::RT_POINTERTO);
          break;
        } // Else overflow
      
      case TT_DECLARATOR: case TT_DECFLAG: case TT_CLASS: case TT_STRUCT: case TT_ENUM: case TT_UNION: 
      case TT_NAMESPACE: case TT_TEMPLATE: case TT_TYPENAME: case TT_TYPEDEF: case TT_USING: case TT_PUBLIC:
      case TT_PRIVATE: case TT_PROTECTED: case TT_COLON: case TT_SCOPE: case TT_RIGHTPARENTH: case TT_RIGHTBRACKET:
      case TT_LEFTBRACE: case TT_RIGHTBRACE: case TT_LESSTHAN:case TT_GREATERTHAN: case TT_TILDE: case TT_EQUALS:
      case TT_COMMA: case TT_SEMICOLON: case TT_STRINGLITERAL: case TT_DECLITERAL: case TT_HEXLITERAL:
      case TT_OCTLITERAL: case TT_ENDOFCODE: case TT_INVALID: default:
          refs.append(postfix);
          refs.append_nest(append);
        return 0;
    }
    token = lex->get_token(herr);
  }
}
