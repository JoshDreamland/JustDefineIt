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
#include "../src/API/jdi.h"
#include "../src/API/AST.h"
#include "../src/API/user_tokens.h"
#include "../src/General/llreader.h"
#include "../src/General/quickstack.h"

using namespace jdi;
using namespace jdip;
int main() {
  read_declarators("test/primitives.txt", UF_PRIMITIVE);
  read_declarators("test/modifiers.txt", UF_FLAG);
  builtin.load_standard_builtins();
  builtin.output_types();
  cout << endl << endl;
  
  cout << "Testing simple macros" << endl;
  builtin.add_macro("scalar_macro","simple value");
  builtin.add_macro_func("simple_function","Takes no parameters");
  builtin.add_macro_func("one_arg_function","x","(1/(1-(x)))",false);
  builtin.add_macro_func("two_arg_function","a","b","(-(b)/(2*(a)))",false);
  builtin.add_macro_func("variadic_three_arg_function","a","b","c","printf(a,b,c)",true);
  builtin.output_macros();
  
  cout << sizeof(jdip::macro_type) << ", " << sizeof(jdip::macro_function) << ", " << sizeof(jdip::macro_scalar) << endl;
  
  cout << endl;
  cout << endl;
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
  
  cout << endl << endl << endl << endl << "==================================================================" << endl;
  cout << endl << "Ignore that shit for now. Test expression evaluator:" << endl << endl;
  
  AST ast;
  ast << create_token_dec_literal("10",2);
  value v = ast.eval();
  cout << v.val.i << endl;
  
  ast.clear();
  ast << create_token_dec_literal("20",2);
  v = ast.eval();
  cout << v.val.i << endl;
  
  ast.clear();
  ast << create_token_dec_literal("20",2);
  ast << create_token_operator("+",1);
  ast << create_token_dec_literal("10",2);
  v = ast.eval();
  cout << v.val.i << endl;
  
  ast.clear();
  ast << create_token_dec_literal("20",2);
  ast << create_token_operator("+",1);
  ast << create_token_dec_literal("10",2);
  ast << create_token_operator("+",1);
  ast << create_token_dec_literal("10",2);
  v = ast.eval();
  cout << v.val.i << endl;
  
  ast.clear();
  ast << create_token_dec_literal("20",2);
  ast << create_token_operator("+",1);
  ast << create_token_dec_literal("40",2);
  ast << create_token_operator("-",1);
  ast << create_token_dec_literal("10",2);
  v = ast.eval();
  cout << v.val.i << endl;
  
  ast.clear();
  ast << create_token_dec_literal("20",2);
  ast << create_token_operator("+",1);
  ast << create_token_dec_literal("40",2);
  ast << create_token_operator("/",1);
  ast << create_token_dec_literal("10",2);
  ast << create_token_operator("<<",2);
  ast << create_token_dec_literal("1",1);
  v = ast.eval();
  cout << v.val.i << endl;
  
  ast.clear();
  ast << create_token_dec_literal("2",1);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_00.svg");
  ast << create_token_operator("<<",2);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_01.svg");
  ast << create_token_dec_literal("1",1);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_02.svg");
  ast << create_token_operator("+",1);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_03.svg");
  ast << create_token_dec_literal("6",1);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_04.svg");
  ast << create_token_operator("/",1);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_05.svg");
  ast << create_token_dec_literal("2",1);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_06.svg");
  ast << create_token_operator("-",1);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_07.svg");
  ast << create_token_dec_literal("2",1);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_08.svg");
  ast << create_token_operator("/",1);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_09.svg");
  ast << create_token_dec_literal("2",1);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_10.svg");
  ast << create_token_operator("<<",2);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_11.svg");
  ast << create_token_dec_literal("8",1);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_12.svg");
  ast << create_token_operator("%",1);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_13.svg");
  ast << create_token_dec_literal("5",1);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_14.svg");
  ast << create_token_operator("+",1);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_15.svg");
  ast << create_token_operator("-",1);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_16.svg");
  ast << create_token_dec_literal("5",1);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_17.svg");
  ast << create_token_operator("/",1);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_18.svg");
  ast << create_token_operator("-",1);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_19.svg");
  ast << create_token_dec_literal("2",1);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_20.svg");
  ast << create_token_operator("==",2);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_21.svg");
  ast << create_token_dec_literal("1",1);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_22.svg");
  ast << create_token_operator("<<",2);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_23.svg");
  ast << create_token_operator("-",1);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_24.svg");
  ast << create_token_dec_literal("2",1);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_25.svg");
  ast << create_token_operator("*",1);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_26.svg");
  ast << create_token_dec_literal("2",1);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_27.svg");
  ast << create_token_operator("*",1);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_28.svg");
  ast << create_token_operator("-",1);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_29.svg");
  ast << create_token_dec_literal("2",1);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_30.svg");
  ast << create_token_operator("+",1);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_31.svg");
  ast << create_token_dec_literal("1",1);
  ast.writeSVG("/home/josh/Desktop/AST_101/AST_32.svg");
  ast.writeSVG("/home/josh/Desktop/AST.svg");
  v = ast.eval();
  cout << v.val.i << endl;
  
  ast.clear();
  ast << create_token_dec_literal("25",2);
  ast.writeSVG("/home/josh/Desktop/AST_102/AST_00.svg");
  ast << create_token_operator("/",1);
  ast.writeSVG("/home/josh/Desktop/AST_102/AST_01.svg");
  ast << create_token_opening_parenth();
  ast.writeSVG("/home/josh/Desktop/AST_102/AST_02.svg");
  ast << create_token_dec_literal("2",1);
  ast.writeSVG("/home/josh/Desktop/AST_102/AST_03.svg");
  ast << create_token_operator("+",1);
  ast.writeSVG("/home/josh/Desktop/AST_102/AST_04.svg");
  ast << create_token_dec_literal("3",1);
  ast.writeSVG("/home/josh/Desktop/AST_102/AST_05.svg");
  ast << create_token_closing_parenth();
  ast.writeSVG("/home/josh/Desktop/AST_102/AST_06.svg");
  ast << create_token_operator("+",1);
  ast.writeSVG("/home/josh/Desktop/AST_102/AST_07.svg");
  ast << create_token_opening_parenth();
  ast.writeSVG("/home/josh/Desktop/AST_102/AST_08.svg");
  ast << create_token_dec_literal("2",1);
  ast.writeSVG("/home/josh/Desktop/AST_102/AST_09.svg");
  ast << create_token_operator("+",1);
  ast.writeSVG("/home/josh/Desktop/AST_102/AST_10.svg");
  ast << create_token_dec_literal("1",1);
  ast.writeSVG("/home/josh/Desktop/AST_102/AST_11.svg");
  ast << create_token_operator("*",1);
  ast.writeSVG("/home/josh/Desktop/AST_102/AST_12.svg");
  ast << create_token_dec_literal("1",1);
  ast.writeSVG("/home/josh/Desktop/AST_102/AST_13.svg");
  ast << create_token_closing_parenth();
  ast.writeSVG("/home/josh/Desktop/AST_102/AST_14.svg");
  ast << create_token_operator("*",1);
  ast.writeSVG("/home/josh/Desktop/AST_102/AST_15.svg");
  ast << create_token_dec_literal("2",1);
  ast.writeSVG("/home/josh/Desktop/AST_102/AST_16.svg");
  ast << create_token_operator("+",1);
  ast.writeSVG("/home/josh/Desktop/AST_102/AST_17.svg");
  ast << create_token_dec_literal("1",1);
  ast.writeSVG("/home/josh/Desktop/AST_102/AST_18.svg");
  v = ast.eval();
  cout << v.val.i << endl;
  
  ast.clear();
  ast << create_token_identifier("a",1);
  ast.writeSVG("/home/josh/Desktop/AST_103/AST_00.svg");
  ast << create_token_operator("=",1);
  ast.writeSVG("/home/josh/Desktop/AST_103/AST_01.svg");
  ast << create_token_dec_literal("1",1);
  ast.writeSVG("/home/josh/Desktop/AST_103/AST_02.svg");
  ast << create_token_operator("-",1);
  ast.writeSVG("/home/josh/Desktop/AST_103/AST_03.svg");
  ast << create_token_dec_literal("1",1);
  ast.writeSVG("/home/josh/Desktop/AST_103/AST_04.svg");
  ast << create_token_operator("?",1);
  ast.writeSVG("/home/josh/Desktop/AST_103/AST_05.svg");
  ast << create_token_identifier("b",1);
  ast.writeSVG("/home/josh/Desktop/AST_103/AST_06.svg");
  ast << create_token_operator("=",1);
  ast.writeSVG("/home/josh/Desktop/AST_103/AST_07.svg");
  ast << create_token_dec_literal("1",1);
  ast.writeSVG("/home/josh/Desktop/AST_103/AST_08.svg");
  ast << create_token_operator("+",1);
  ast.writeSVG("/home/josh/Desktop/AST_103/AST_09.svg");
  ast << create_token_dec_literal("2",1);
  ast.writeSVG("/home/josh/Desktop/AST_103/AST_10.svg");
  ast << create_token_operator("<<",2);
  ast.writeSVG("/home/josh/Desktop/AST_103/AST_11.svg");
  ast << create_token_dec_literal("2",1);
  ast.writeSVG("/home/josh/Desktop/AST_103/AST_12.svg");
  ast << create_token_colon();
  ast.writeSVG("/home/josh/Desktop/AST_103/AST_13.svg");
  ast << create_token_identifier("c",1);
  ast.writeSVG("/home/josh/Desktop/AST_103/AST_14.svg");
  ast << create_token_operator("=",1);
  ast.writeSVG("/home/josh/Desktop/AST_103/AST_15.svg");
  ast << create_token_dec_literal("3",1);
  ast.writeSVG("/home/josh/Desktop/AST_103/AST_16.svg");
  ast << create_token_operator("+",1);
  ast.writeSVG("/home/josh/Desktop/AST_103/AST_17.svg");
  ast << create_token_dec_literal("4",1);
  ast.writeSVG("/home/josh/Desktop/AST_103/AST_18.svg");
  v = ast.eval();
  cout << v.val.i << endl;
  
  return 0;
}
