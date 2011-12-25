/**
 * @file  debug_macros.cpp
 * @brief Source implementing the conditional macros for tracking code positions.
 * 
 * This file implements the ass-end of the debug macros.
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

#ifdef DEBUG_MODE
  #include <stack>
  #include <cstdio>
  #include "debug_macros.h"
  using namespace std;
  
  string loopiters[20];
  void log_loop_iteration(const char* fn, int ln) {
    for (int i = 1; i < 20; i++)
      loopiters[i-1] = loopiters[i];
    char buf[4096];
    sprintf(buf,"Loop iteration, %s:%d\n", fn, ln);
    loopiters[19] = buf;
    //puts(buf);
  }
  
  static stack<string> stack_trace;
  stack_enter_obj::stack_enter_obj(const char* fn, int ln, const char* fnc) {
    char buf[4096];
    sprintf(buf,"%s, %s:%d\n", fnc, fn, ln);
    stack_trace.push(string(buf));
    //puts(buf);
  }
  stack_enter_obj::~stack_enter_obj() {
    stack_trace.pop();
  }
#endif
