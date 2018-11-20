/**
 * @file  error_reporting.cpp
 * @brief Source implementing the default error handling class.
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

#include <cstdio>
#include <API/error_reporting.h>

#define null_term(string_view) (std::string(string_view).c_str())

namespace jdi {
  void default_error_handler::error(std::string_view err, std::string_view filename, int line, int pos) {
    if (!filename.length())
      fprintf(stderr, "ERROR: %s\n", null_term(err));
    if (line == -1)
      fprintf(stderr, "ERROR(%s): %s\n", null_term(filename), null_term(err));
    else if (pos == -1)
      fprintf(stderr, "ERROR(%s:%d): %s\n", null_term(filename), line, null_term(err));
    else
      fprintf(stderr, "ERROR(%s,%d,%d): %s\n", null_term(filename), line, pos, null_term(err));
    ++error_count;
  }
  void default_error_handler::warning(std::string_view err, std::string_view filename, int line, int pos) {
    if (!filename.length())
      fprintf(stderr, "Warning: %s\n", null_term(err));
    if (line == -1)
      fprintf(stderr, "Warning(%s): %s\n", null_term(filename), null_term(err));
    else if (pos == -1)
      fprintf(stderr, "Warning(%s:%d): %s\n", null_term(filename), line, null_term(err));
    else
      fprintf(stderr, "Warning(%s,%d,%d): %s\n", null_term(filename), line, pos, null_term(err));
    ++warning_count;
  }
  
  default_error_handler::default_error_handler(): error_count(0), warning_count(0) {}

  /// The instance of \c default_error_handler to which \c def_error_handler will point.
  static default_error_handler deh_instance;
  default_error_handler *def_error_handler = &deh_instance;

  error_handler::~error_handler() {}
}
