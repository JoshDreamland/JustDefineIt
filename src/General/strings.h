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
#include <string_view>

namespace jdi {

  template<typename T> std::string to_string(T x) { return std::to_string(x); }
  static inline std::string to_string(const std::string &x) { return x; }
  static inline std::string to_string(std::string_view x) {
    return std::string(x);
  }

  template <typename... Args>
  std::string format(std::string_view text, Args... args) {
    std::string res;
    std::string strs[] = { jdi::to_string(args)... };

    int cap = text.length();
    for (const std::string str : strs) { cap += str.length() - 2; }
    if (cap > 0) res.reserve(cap);  // Should always be true if format str is correct.

    size_t p = 0;
    for (const std::string str : strs) {
      const size_t pn = text.find("%s", p);
      if (pn == std::string::npos) break;
      res += text.substr(p, pn - p);
      res += str;
      p = pn + 2;
    }
    if (p != std::string::npos) res += text.substr(p);
    return res;
  }

  static inline std::string format(std::string_view text) {
    return std::string(text);
  }

  static inline std::string quote(std::string_view str) {
    std::string res;
    res.reserve(str.length() * 2 + 2);
    res += '"';
    for (char c : str) switch (c) {
      case '\r': res += "\\r"; break;
      case '\n': res += "\\n"; break;
      case '\\': res += "\\\\"; break;
      case '\"': res += "\\\""; break;
      case '\'': res += "\\\'"; break;
      default: res += c;
    }
    res += '"';
    return res;
  }
}

#endif
