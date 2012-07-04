/**
 * @file symbols.cpp
 * @brief Source populating symbol tables for AST building.
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

#include <cstdio>
#include "symbols.h"
#include <Storage/value_funcs.h>

namespace jdip {
  symbol_table symbols;

  symbol::symbol() {}
  symbol::symbol(unsigned char t, unsigned char p):
    type(t), prec_binary(t&ST_BINARY? p:0), prec_unary_pre(t&ST_UNARY_PRE? p:0), prec_unary_post(t&ST_UNARY_POST? p:0),
    operate(NULL), operate_unary_pre(NULL), operate_unary_post(NULL) {}
  symbol::symbol(unsigned char t, unsigned char p, value(*o)(const value&, const value&)):
    type(t), prec_binary(p), prec_unary_pre(0), prec_unary_post(0),
    operate(o), operate_unary_pre(NULL), operate_unary_post(NULL) {}
  symbol::symbol(unsigned char t, unsigned char p, value(*ou)(const value&)):
    type(t), prec_binary(0), prec_unary_pre(t&ST_UNARY_PRE? p:0), prec_unary_post(t&ST_UNARY_POST? p:0),
    operate(NULL), operate_unary_pre(t&ST_UNARY_PRE? ou:NULL), operate_unary_post(t&ST_UNARY_POST? ou:NULL) {}
  symbol& symbol::operator|=(const symbol& other) {
    type |= other.type;
    prec_binary |= other.prec_binary;
    prec_unary_pre |= other.prec_unary_pre;
    prec_unary_post |= other.prec_unary_post;
    if (!operate) operate = other.operate;
    if (!operate_unary_pre) operate_unary_pre = other.operate_unary_pre;
    if (!operate_unary_post) operate_unary_post = other.operate_unary_post;
    return *this;
  }
}

using namespace jdip;

/// Constructor designed to circumvent C++'s lack of static initializer blocks; will be run at program start to populate symbols.
/// Simply maps all the symbols with their AST generation and evaluation information.
symbol_table::symbol_table()
{
  int prec = PRECEDENCE_MAX; // Count down precedence levels to avoid accidental decrements.
  
  prec--;
  symbols["::"] = symbol(ST_BINARY, prec);
  
  prec--;
  symbols["++"] = symbol(ST_UNARY_POST,prec,value_unary_increment);
  symbols["--"] = symbol(ST_UNARY_POST,prec,value_unary_decrement);
  symbols["("]  = symbol(ST_BINARY,prec);
  symbols["["]  = symbol(ST_BINARY,prec);
  symbols["."]  = symbol(ST_BINARY,prec);
  symbols["->"] = symbol(ST_BINARY,prec);
  
  prec--;
  symbols["++"] |= symbol(ST_UNARY_PRE,prec,value_unary_increment);
  symbols["--"] |= symbol(ST_UNARY_PRE,prec,value_unary_decrement);
  symbols["+"]   = symbol(ST_UNARY_PRE,prec,value_unary_positive);
  symbols["-"]   = symbol(ST_UNARY_PRE,prec,value_unary_negative);
  symbols["!"]   = symbol(ST_UNARY_PRE,prec,value_unary_not);
  symbols["~"]   = symbol(ST_UNARY_PRE,prec,value_unary_negate);
  symbols["*"]   = symbol(ST_UNARY_PRE,prec,value_unary_dereference);
  symbols["&"]   = symbol(ST_UNARY_PRE,prec,value_unary_reference);
  symbols["sizeof"] = symbol(ST_UNARY_PRE,prec);
  symbols["new"]    = symbol(ST_UNARY_PRE,prec);
  symbols["delete"] = symbol(ST_UNARY_PRE,prec);
  
  prec--;
  symbols[".*"]  = symbol(ST_UNARY_PRE | ST_UNARY_POST, prec);
  symbols["->*"] = symbol(ST_UNARY_PRE | ST_UNARY_POST, prec);
  
  prec--;
  symbols["*"] |= symbol(ST_BINARY,prec,values_multiply);
  symbols["/"]  = symbol(ST_BINARY,prec,values_divide);
  symbols["%"]  = symbol(ST_BINARY,prec,values_modulo);
  
  prec--;
  symbols["+"] |= symbol(ST_BINARY,prec,values_add);
  symbols["-"] |= symbol(ST_BINARY,prec,values_subtract);
  
  prec--;
  symbols["<<"] = symbol(ST_BINARY,prec,values_lshift);
  symbols[">>"] = symbol(ST_BINARY,prec,values_rshift);
  
  prec--;
  symbols["<"]  = symbol(ST_BINARY,prec,values_less);
  symbols[">"]  = symbol(ST_BINARY,prec,values_greater);
  symbols["<="] = symbol(ST_BINARY,prec,values_less_or_equal);
  symbols[">="] = symbol(ST_BINARY,prec,values_greater_or_equal);
  
  prec--;
  symbols["=="]  = symbol(ST_BINARY,prec,values_equal);
  symbols["!="]  = symbol(ST_BINARY,prec,values_notequal);
  
  prec--; symbols["&"] |= symbol(ST_BINARY,prec,values_bitand);
  prec--; symbols["^"]  = symbol(ST_BINARY,prec,values_bitxor);
  prec--; symbols["|"]  = symbol(ST_BINARY,prec,values_bitor);
  
  prec--; symbols["&&"] = symbol(ST_BINARY,prec,values_booland);
  prec--; symbols["^^"] = symbol(ST_BINARY,prec,values_boolxor);
  prec--; symbols["||"] = symbol(ST_BINARY,prec,values_boolor);
  
  prec--; symbols["?"]  = symbol(ST_TERNARY | ST_RTL_PARSED,prec);
  
  prec--;
  symbols["="]   = symbol(ST_BINARY | ST_RTL_PARSED,prec, values_latter);
  symbols["+="]  = symbol(ST_BINARY | ST_RTL_PARSED,prec);
  symbols["-="]  = symbol(ST_BINARY | ST_RTL_PARSED,prec);
  symbols["*="]  = symbol(ST_BINARY | ST_RTL_PARSED,prec);
  symbols["%="]  = symbol(ST_BINARY | ST_RTL_PARSED,prec);
  symbols["/="]  = symbol(ST_BINARY | ST_RTL_PARSED,prec);
  symbols["&="]  = symbol(ST_BINARY | ST_RTL_PARSED,prec);
  symbols["^="]  = symbol(ST_BINARY | ST_RTL_PARSED,prec);
  symbols["|="]  = symbol(ST_BINARY | ST_RTL_PARSED,prec);
  symbols["<<="] = symbol(ST_BINARY | ST_RTL_PARSED,prec);
  symbols[">>="] = symbol(ST_BINARY | ST_RTL_PARSED,prec);
  
  prec--;
  symbols[","]  = symbol(ST_BINARY,prec,values_latter);
  
  if (prec != 1)
    perror("INTERNAL ERROR. INCORRECT PRECENDENCE COUNT GIVEN.\n");
}
