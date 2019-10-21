/**
 * @file  macros.cpp
 * @brief System source implementing functions for parsing macros.
 * 
 * In general, the implementation is unremarkable.
 * 
 * @section License
 * 
 * Copyright (C) 2011-2012 Josh Ventura
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

#include <cstring>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

#include "macros.h"
#include <General/parse_basics.h>
#include <General/debug_macros.h>
#include <System/lex_cpp.h>
using namespace jdi;

string macro_type::NameAndPrototype() const {
  string res = name;
  if (is_function) {
    res += "(";
    for (size_t i = 0; i < params.size(); i++)
      res += params[i]
          + (i + 1 < params.size() ? ", " : is_variadic ? "..." : "");
    res += ")";
  }
  return res;
}

string macro_type::toString() const {
  string res = "#define " + NameAndPrototype() + " \\\n";
  for (size_t i = 0; i < value.size(); ++i)
    res += "  " + value[i].to_string() + (i + 1 < value.size()? "\\\n" : "");
  return res;
}

vector<macro_type::FuncComponent> macro_type::componentize(
    const token_vector &tokens, const vector<string> &params,
    error_handler *herr) {
  vector<macro_type::FuncComponent> res;
  std::unordered_map<std::string_view, size_t> params_by_name;
  for (size_t i = 0; i < params.size(); ++i) params_by_name[params[i]] = i;
  size_t e = 0;
  for (size_t i = 0; i < tokens.size(); ++i) {
    if (tokens[i].type == TTM_CONCAT) {
      if (!i) {
        herr->error(tokens[i], "Concatenation marker cannot appear at either "
                               "end of a replacement list.");
        continue;
      }
      if (e != i) res.push_back(FuncComponent::TokenSpan{e, i});
      res.push_back(FuncComponent::Paste{});
      e = i + 1;
    } else if (tokens[i].type == TT_IDENTIFIER) {
      auto found = params_by_name.find(tokens[i].content.view());
      if (found != params_by_name.end()) {
        if (e != i) res.push_back(FuncComponent::TokenSpan{e, i});
        if ((i + 1 < tokens.size() && tokens[i + 1].type == TTM_CONCAT) ||
            (i > 0 && tokens[i - 1].type == TTM_CONCAT)) {
          res.push_back(FuncComponent::RawArgument{found->second});
        } else {
          res.push_back(FuncComponent::ExpandedArgument{found->second});
        }
        e = i + 1;
      }
    } else if (tokens[i].type == TTM_TOSTRING) {
      if (i + 1 >= tokens.size() || tokens[i + 1].type != TT_IDENTIFIER) {
        herr->error(tokens[i], "# must be followed by a parameter name");
        continue;
      }
      if (e != i) res.push_back(FuncComponent::TokenSpan{e, i});
      auto found = params_by_name.find(tokens[++i].content.view());
      if (found == params_by_name.end()) {
        herr->error(tokens[i], "# must be followed by a parameter name; " +
                    tokens[i].content.toString() + " is not a parameter");
        continue;
      }
      res.push_back(FuncComponent::Stringify{found->second});
      e = i + 1;
    }
  }
  if (e < tokens.size())
    res.push_back(FuncComponent::TokenSpan{e, tokens.size()});
  return res;
}

static token_t paste_tokens(token_t left, token_t right, error_handler *herr) {
  string buf = left.content.toString() + right.content.toString();
  llreader read("token concatenation", buf, false);
  token_t res = read_token(read, herr);
  if (!read.eof()) {
    herr->error(left, "Concatenation of `%s` and `%s` does not yield a coherent"
                " token.", left.to_string(), right.to_string());
  }
  return res;
}

token_vector macro_type::evaluate_concats(token_vector &&replacement_list,
                                          error_handler *herr) {
  size_t left = 0;
  for (size_t i = 1; i < replacement_list.size(); ++i) {
    if (replacement_list[i].type == TTM_CONCAT) {
      if (++i >= replacement_list.size()) break;
      replacement_list[left] =
          paste_tokens(replacement_list[left], replacement_list[i],herr);
    } else if (++left < i) {
      replacement_list[left] = replacement_list[i];
    }
  }
  // Conditional handles the zero-size case.
  if (++left < replacement_list.size()) replacement_list.resize(left);
  return replacement_list;
}

static void append_or_paste(token_vector &dest,
                            token_vector::const_iterator begin,
                            token_vector::const_iterator end,
                            bool paste, error_handler *herr) {
  if (begin == end) return;
  if (paste) {
    token_t &left = dest.back();
    left = paste_tokens(left, *begin++, herr);
  }
  dest.insert(dest.end(), begin, end);
}

token_vector macro_type::substitute_and_unroll(
    const vector<token_vector> &args, const vector<token_vector> &args_evald,
    error_handler *herr) const {
  token_vector res;
  bool paste_next = false;
  if (args.size() < params.size()) {
    herr->error("Too few arguments to macro " + NameAndPrototype() +
                ": wanted " + std::to_string(params.size()) +
                ", got " + std::to_string(args.size()));
  }
  if (args.size() > params.size()) {
    if (!is_variadic) {
      herr->error("Too many arguments to macro " + NameAndPrototype() +
                  ": wanted " + std::to_string(params.size()) +
                  ", got " + std::to_string(args.size()));
    } else if (args.size() != params.size() + 1) {
      herr->error("Internal error: variadic macro passed too many arguments");
    }
  }
  for (const FuncComponent &part : parts) {
    switch (part.tag) {
      case FuncComponent::TOKEN_SPAN:
        append_or_paste(res, value.begin() + part.token_span.begin,
                             value.begin() + part.token_span.end,
                        paste_next, herr);
        paste_next = false;
        break;
      case FuncComponent::RAW_ARGUMENT:
      case FuncComponent::EXPANDED_ARGUMENT:
      case FuncComponent::STRINGIFY: {
        const size_t ind = part.raw_expanded_or_stringify_argument.index;
        if (ind >= args.size()) {
          if (ind >= params.size()) {
            herr->error(
                "Internal error: Macro function built with bad argument "
                "references. Index " + std::to_string(ind) + " out of bounds "
                "(only " + std::to_string(params.size()) + " params defined).");
          }
          paste_next = false;
          continue;
        }
        if (part.tag == FuncComponent::STRINGIFY) {
          string str;
          for (const token_t &tok : args[ind])
            str += tok.content.toString();
          str = quote(str);
          string name_str = "#" + params[ind];
          token_vector vec{token_t(TT_STRINGLITERAL, name_str.c_str(), 0, 0, std::move(str))};
          append_or_paste(res, vec.begin(), vec.end(), paste_next, herr);
          paste_next = false;
          break;
        }
        const token_vector &vec =
            part.tag == FuncComponent::EXPANDED_ARGUMENT
                ? args_evald[ind] : args[ind];
        append_or_paste(res, vec.begin(), vec.end(), paste_next, herr);
        paste_next = false;
        break;
      }
      case FuncComponent::PASTE:
        paste_next = true;
        break;
      case FuncComponent::VA_ARGS:
        if (args.size() == params.size() + 1) {
          append_or_paste(res, args[params.size()].begin(),
                               args[params.size()].end(),
                          paste_next, herr);
        } else {
          token_vector empty;
          append_or_paste(res, empty.begin(), empty.end(), paste_next, herr);
        }
        break;
      case FuncComponent::VA_OPT: {
        token_vector opt;
        if (args.size() == params.size() + 1 && !args[params.size()].empty()) {
          opt.push_back(token_t(TT_COMMA, "__VA_OPT__", 0, 0, ",", 1));
        } else {
          token_vector empty;
          append_or_paste(res, empty.begin(), empty.end(), paste_next, herr);
        }
        break;
      }
      default:
        herr->error("Internal error: Macro function component unknown...");
    }
  }
  return res;
}
