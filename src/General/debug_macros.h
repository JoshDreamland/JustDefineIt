/**
 * @file  debug_macros.h
 * @brief A header declaring conditional macros for tracking code position.
 * 
 * This file declares macros for reporting loop iterations, stack traces, and
 * other information that would otherwise need to be acquired via a debugger.
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
  #include <string>
  /** Conditionally track loop iteration history for finding infinite loops. **/
  #define loop_tracer() log_loop_iteration(__FILE__,__LINE__)
  /** Completely unstable function for tracking loop iteration **/
  void log_loop_iteration(const char*,int);
  /** Conditionally track call stack information for locating mishaps. **/
  #define stack_tracer(fname) stack_enter_obj seobj(__FILE__, __LINE__, "" fname);
  /** Completely unstable function for tracking call stack information. **/
  struct stack_enter_obj { stack_enter_obj(const char*, int, const char*); ~stack_enter_obj(); };
#else
  #define loop_tracer()
  #define stack_tracer(fname)
#endif
