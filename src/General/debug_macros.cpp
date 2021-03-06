/**
 * @file  debug_macros.cpp
 * @brief Source implementing the conditional macros for parser debugging.
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
 * JustDefineIt is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for details.
 *
 * You should have received a copy of the GNU General Public License along with
 * JustDefineIt. If not, see <http://www.gnu.org/licenses/>.
**/

#ifdef DEBUG_MODE
#include <string>
#include <cstdio>
#include <filesystem>
#include <API/AST.h>
#include "debug_macros.h"

using std::string;

#include <sys/stat.h>
void render_ast_nd(jdi::AST& ast, std::string cat) {
  std::filesystem::path fullp = DEBUG_OUTPUT_PATH "/AST_Renders/";
  fullp /= cat;  // The full path to which this AST will be rendered.

  static unsigned ast_rn = 0;
  if (!ast_rn) {
    std::filesystem::create_directories(fullp);
  }
  char fn[32];
  sprintf(fn,"ast_%08u.svg", ast_rn++);
  fullp /= fn;

  ast.writeSVG(fullp.c_str());
}

#ifdef RENDER_ASTS
  void render_ast(jdi::AST& ast, std::string cat)
  { render_ast_nd(ast, cat); }
#endif

#endif
