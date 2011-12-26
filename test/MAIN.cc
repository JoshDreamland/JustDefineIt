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
#include "../src/General/llreader.h"
#include "../src/General/quickstack.h"

using namespace jdi;
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
}
