/* Copyright (C) 2011 Josh Ventura
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
*/

#include <cstdio>
#include <iostream>
using namespace std;
#include <API/jdi.h>
#include <API/AST.h>
#include <API/user_tokens.h>
#include <General/llreader.h>
#include <General/quickstack.h>
#include "debug_lexer.h"

using namespace jdi;
using namespace jdip;

void test_expression_evaluator();

static void putcap(string cap) {
  cout << endl << endl << endl << endl;
  cout << "============================================================================" << endl;
  cout << "=: " << cap << " :="; for (int i = 70 - cap.length(); i > 0; i--) fputc('=',stdout); cout << endl;
  cout << "============================================================================" << endl << endl;
}

int main() {
  add_gnu_declarators();
  builtin.load_standard_builtins();
  builtin.output_types();
  cout << endl << endl;
  
  putcap("Test simple macros");
  builtin.add_macro("scalar_macro","simple value");
  builtin.add_macro_func("simple_function","Takes no parameters");
  builtin.add_macro_func("one_arg_function","x","(1/(1-(x)))",false);
  builtin.add_macro_func("two_arg_function","a","b","(-(b)/(2*(a)))",false);
  builtin.add_macro_func("variadic_three_arg_function","a","b","c","printf(a,b,c)",true);
  builtin.output_macros();
  
  putcap("Metrics");
  cout << "sizeof(jdip::macro_type):       " << sizeof(jdip::macro_type) << endl
       << "sizeof(jdip::macro_function):   " << sizeof(jdip::macro_function) << endl
       << "sizeof(jdip::macro_scalar):     " << sizeof(jdip::macro_scalar) << endl;
  
  test_expression_evaluator();
  
  
  putcap("Test parser");
  llreader f("test/test.cc");
  if (f.is_open())
  {
    context enigma;
    if (enigma.parse_C_stream(f))
      cout << "ERROR: " << enigma.get_last_error() << endl;
    enigma.output_definitions();
  }
  else
    cout << "Failed to open file for parsing!" << endl;
  
  return 0;
}

void test_expression_evaluator() {
  putcap("Test expression evaluator");
  
  AST ast;
  debug_lexer dlex;
  dlex << create_token_dec_literal("10",2);
  ast.parse_expression(&dlex);
  value v = ast.eval();
  ast.writeSVG("/home/josh/Desktop/RecursiveAST/AST_00.svg");
  cout << v.val.i << endl;
  
  ast.clear(); dlex.clear();
  dlex << create_token_dec_literal("20",2);
  ast.parse_expression(&dlex);
  v = ast.eval();
  ast.writeSVG("/home/josh/Desktop/RecursiveAST/AST_01.svg");
  cout << v.val.i << endl;
  
  ast.clear(); dlex.clear();
  dlex << create_token_dec_literal("20",2);
  dlex << create_token_operator("+",1);
  dlex << create_token_dec_literal("10",2);
  ast.parse_expression(&dlex);
  v = ast.eval();
  ast.writeSVG("/home/josh/Desktop/RecursiveAST/AST_02.svg");
  cout << v.val.i << endl;
  
  ast.clear(); dlex.clear();
  dlex << create_token_dec_literal("20",2);
  dlex << create_token_operator("+",1);
  dlex << create_token_dec_literal("10",2);
  dlex << create_token_operator("+",1);
  dlex << create_token_dec_literal("10",2);
  ast.parse_expression(&dlex);
  ast.writeSVG("/home/josh/Desktop/RecursiveAST/AST_03.svg");
  v = ast.eval();
  cout << v.val.i << endl;
  
  ast.clear(); dlex.clear();
  dlex << create_token_dec_literal("20",2);
  dlex << create_token_operator("+",1);
  dlex << create_token_dec_literal("40",2);
  dlex << create_token_operator("-",1);
  dlex << create_token_dec_literal("10",2);
  ast.parse_expression(&dlex);
  ast.writeSVG("/home/josh/Desktop/RecursiveAST/AST_04.svg");
  v = ast.eval(); dlex.clear();
  cout << v.val.i << endl;
  
  ast.clear(); dlex.clear();
  dlex << create_token_dec_literal("20",2);
  dlex << create_token_operator("+",1);
  dlex << create_token_dec_literal("40",2);
  dlex << create_token_operator("/",1);
  dlex << create_token_dec_literal("4",1);
  dlex << create_token_operator("<<",2);
  dlex << create_token_dec_literal("1",1);
  token_t token;
  ast.parse_expression(&dlex);
  ast.writeSVG("/home/josh/Desktop/RecursiveAST/AST_05.svg");
  v = ast.eval();
  cout << v.val.i << endl;
  
  ast.clear(); dlex.clear();
  dlex << create_token_dec_literal("2",1);
  dlex << create_token_operator("<<",2);
  dlex << create_token_dec_literal("1",1);
  dlex << create_token_operator("+",1);
  dlex << create_token_dec_literal("6",1);
  dlex << create_token_operator("/",1);
  dlex << create_token_dec_literal("2",1);
  dlex << create_token_operator("-",1);
  dlex << create_token_dec_literal("2",1);
  dlex << create_token_operator("/",1);
  dlex << create_token_dec_literal("2",1);
  dlex << create_token_operator("<<",2);
  dlex << create_token_dec_literal("8",1);
  dlex << create_token_operator("%",1);
  dlex << create_token_dec_literal("5",1);
  dlex << create_token_operator("+",1);
  dlex << create_token_operator("-",1);
  dlex << create_token_dec_literal("5",1);
  dlex << create_token_operator("/",1);
  dlex << create_token_operator("-",1);
  dlex << create_token_dec_literal("2",1);
  dlex << create_token_operator("==",2);
  dlex << create_token_dec_literal("1",1);
  dlex << create_token_operator("<<",2);
  dlex << create_token_operator("-",1);
  dlex << create_token_dec_literal("2",1);
  dlex << create_token_operator("*",1);
  dlex << create_token_dec_literal("2",1);
  dlex << create_token_operator("*",1);
  dlex << create_token_operator("-",1);
  dlex << create_token_dec_literal("2",1);
  dlex << create_token_operator("+",1);
  dlex << create_token_dec_literal("1",1);
  ast.parse_expression(&dlex);
  ast.writeSVG("/home/josh/Desktop/RecursiveAST/AST_06.svg");
  v = ast.eval();
  cout << v.val.i << endl;
  
  ast.clear(); dlex.clear();
  dlex << create_token_dec_literal("25",2);
  dlex << create_token_operator("/",1);
  dlex << create_token_opening_parenth();
  dlex << create_token_dec_literal("2",1);
  dlex << create_token_operator("+",1);
  dlex << create_token_dec_literal("3",1);
  dlex << create_token_closing_parenth();
  dlex << create_token_operator("+",1);
  dlex << create_token_opening_parenth();
  dlex << create_token_dec_literal("2",1);
  dlex << create_token_operator("+",1);
  dlex << create_token_dec_literal("1",1);
  dlex << create_token_operator("*",1);
  dlex << create_token_dec_literal("1",1);
  dlex << create_token_closing_parenth();
  dlex << create_token_operator("*",1);
  dlex << create_token_dec_literal("2",1);
  dlex << create_token_operator("+",1);
  dlex << create_token_dec_literal("1",1);
  ast.parse_expression(&dlex);
  ast.writeSVG("/home/josh/Desktop/RecursiveAST/AST_07.svg");
  v = ast.eval();
  cout << v.val.i << endl;
  
  ast.clear();
  dlex << create_token_identifier("a",1);
  dlex << create_token_operator("=",1);
  dlex << create_token_dec_literal("2",1);
  dlex << create_token_operator("==",2);
  dlex << create_token_dec_literal("1",1);
  dlex << create_token_operator("+",1);
  dlex << create_token_dec_literal("1",1);
  dlex << create_token_operator("?",1);
  dlex << create_token_identifier("b",1);
  dlex << create_token_operator("=",1);
  dlex << create_token_dec_literal("15",2);
  dlex << create_token_operator("*",1);
  dlex << create_token_dec_literal("8",1);
  dlex << create_token_operator("+",1);
  dlex << create_token_dec_literal("3",1);
  dlex << create_token_colon();
  dlex << create_token_identifier("c",1);
  dlex << create_token_operator("=",1);
  dlex << create_token_dec_literal("3",1);
  dlex << create_token_operator("+",1);
  dlex << create_token_dec_literal("4",1);
  ast.parse_expression(&dlex);
  ast.writeSVG("/home/josh/Desktop/RecursiveAST/AST_08.svg");
  v = ast.eval();
  cout << v.val.i << endl;
}
