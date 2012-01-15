/**
 * @file symbols.cpp
 * @brief Source populating symbol tables for AST building.
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

#include "symbols.h"
#include <Storage/value_funcs.h>

namespace jdip {
  symbol_table symbols;

  symbol::symbol() {}
  symbol::symbol(short t, short p): type(t), prec(p), operate(NULL), operate_unary(NULL) {} ///< Operation-free constructor.
  symbol::symbol(short t, short p, value(*o)(const value&, const value&)): type(t), prec(p), operate(o), operate_unary(NULL) {} ///< 
  symbol::symbol(short t, short p, value(*o)(const value&, const value&), value(*ou)(const value&)): type(t), prec(p), operate(o), operate_unary(ou) {}
  symbol::symbol(short t, short p, value(*ou)(const value&)): type(t), prec(p), operate(NULL), operate_unary(ou) {}
}

using namespace jdip;

/**
  A structure designed to circumvent C++'s lack of static initializer blocks.
  Simply maps all the symbols with their AST generation and evaluation information.
**/
static struct map_symbols_ {
  /// Constructor will be run at program start to populate symbols.
  map_symbols_ () {
    symbols["::"] = symbol(ST_BINARY, 1);
    
    symbols["["]  = symbol(ST_BINARY,2);
    symbols["("]  = symbol(ST_BINARY | ST_UNARY_PRE,2);
    symbols["."]  = symbol(ST_BINARY,2);
    symbols["->"] = symbol(ST_BINARY,2);
    
    symbols["++"] = symbol(ST_UNARY_PRE | ST_UNARY_POST,2,value_unary_increment);
    symbols["--"] = symbol(ST_UNARY_PRE | ST_UNARY_POST,2,value_unary_decrement);
    symbols["!"] = symbol(ST_UNARY_PRE | ST_UNARY_POST,2,value_unary_increment);
    symbols["~"] = symbol(ST_UNARY_PRE | ST_UNARY_POST,2,value_unary_increment);
    
    symbols[".*"] = symbol(ST_UNARY_PRE | ST_UNARY_POST,3);
    symbols["->*"] = symbol(ST_UNARY_PRE | ST_UNARY_POST,3);
    
    symbols["*"]  = symbol(ST_UNARY_PRE | ST_BINARY,4,values_multiply,value_unary_dereference);
    symbols["/"]  = symbol(ST_BINARY,4,values_divide);
    symbols["%"]  = symbol(ST_BINARY,4,values_modulo);
    
    symbols["+"]  = symbol(ST_UNARY_PRE | ST_BINARY,5,values_add,value_unary_positive);
    symbols["-"]  = symbol(ST_UNARY_PRE | ST_BINARY,5,values_subtract,value_unary_negative);
    
    symbols["<<"]  = symbol(ST_BINARY,6,values_lshift);
    symbols[">>"]  = symbol(ST_BINARY,6,values_rshift);
    
    symbols["<"]  = symbol(ST_BINARY,7,values_less);
    symbols[">"]  = symbol(ST_BINARY,7,values_greater);
    symbols["<="]  = symbol(ST_BINARY,7,values_less_or_equal);
    symbols[">="]  = symbol(ST_BINARY,7,values_greater_or_equal);
    
    symbols["=="]  = symbol(ST_BINARY,8,values_equal);
    symbols["!="]  = symbol(ST_BINARY,8,values_notequal);
    
    symbols["&"]  = symbol(ST_UNARY_PRE | ST_BINARY,9,value_unary_reference);
    symbols["^"]  = symbol(ST_BINARY,10);
    symbols["|"]  = symbol(ST_BINARY,11);
    
    symbols["&&"] = symbol(ST_UNARY_PRE | ST_BINARY,12);
    symbols["^^"] = symbol(ST_BINARY,13);
    symbols["||"] = symbol(ST_BINARY,14);
    
    symbols["?"]  = symbol(ST_TERNARY | ST_RTL_PARSED,15);
    
    symbols["="]   = symbol(ST_BINARY | ST_RTL_PARSED,16, values_latter);
    symbols["+="]  = symbol(ST_BINARY | ST_RTL_PARSED,16);
    symbols["-="]  = symbol(ST_BINARY | ST_RTL_PARSED,16);
    symbols["*="]  = symbol(ST_BINARY | ST_RTL_PARSED,16);
    symbols["%="]  = symbol(ST_BINARY | ST_RTL_PARSED,16);
    symbols["/="]  = symbol(ST_BINARY | ST_RTL_PARSED,16);
    symbols["&="]  = symbol(ST_BINARY | ST_RTL_PARSED,16);
    symbols["^="]  = symbol(ST_BINARY | ST_RTL_PARSED,16);
    symbols["|="]  = symbol(ST_BINARY | ST_RTL_PARSED,16);
    symbols["<<="] = symbol(ST_BINARY | ST_RTL_PARSED,16);
    symbols[">>="] = symbol(ST_BINARY | ST_RTL_PARSED,16);
    
    symbols[","]  = symbol(ST_BINARY,17,values_latter);
  }
} map_symbols;
