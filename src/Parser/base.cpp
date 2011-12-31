/**
 * @file  base.cpp
 * @brief The source implementing the top-level function of the parser.
 * 
 * Practically no work is done by this file; it's mostly the front-end, but such
 * is usually the case in a recursive-descent scheme. The job of this file is
 * simply to mark that we're parsing and hand the baton to the scope handler.
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

#include <fstream>
#include "../System/context.h"
#include "../System/token.h"
#include "../General/debug_macros.h"
#include "parse_context.h"
#include "bodies.h"
using namespace std;
using namespace jdip;

/**
  @brief Parse an input stream for definitions.
  
  This function's only purpose is to make a call to fetch the next token, figure
  out what it represents, and dispatch a new function to deal with it. The vast
  majority of the work is done in the function that grabs the next token.
  
  @param cfile  The stream to be read in.
**/
int jdi::context::parse_C_stream(llreader &cfile)
{
  stack_tracer("void jdi::context::parse_C_stream(llreader &cfile)");
  
  if (pc) {
    if (pc->active) {
      pc->error = "STILL PARSING";
      return -1;
    }
    delete pc;
  }
  pc = new parse_context();
  
  token_t dummy;
  int res = ((context_parser*)this)->handle_scope(cfile, global, dummy);
  pc->active = false;
  return res;
}
