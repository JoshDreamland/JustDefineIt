/**
 * @file  error_reporting.h
 * @brief Header declaring a base class for error and warning reporting.
 *
 * Also defines a default error handling class, which shall write all warnings
 * and errors to stderr.
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
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * JustDefineIt. If not, see <http://www.gnu.org/licenses/>.
**/

#ifndef STRINGS__H
#define STRINGS__H

#include <string>

namespace jdi {

  template <typename... Args>
  std::string format(std::string_view text, Args... args) {
    std::string res;
    std::string strs[] = { std::to_string(args)... };

    int cap = text.length();
    for (const std::string str : strs) { cap += str.length() - 2; }
    if (cap > 0) res.reserve(cap);  // Should always be true if format str is correct.

    size_t p = 0;
    for (const std::string str : strs) {
      const size_t pp = p;
      p = text.find("%s", p);
      if (p == std::string::npos) {
        res += text.substr(pp);
        break;
      }
      res += text.substr(pp, p - pp);
      res += str;
    }
    if (p != std::string::npos) res += text.substr(p);
    return res;
  }

}

#endif
