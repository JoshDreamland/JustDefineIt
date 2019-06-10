/**
 * @file lex_cpp.cpp
 * @brief Source implementing the C++ \c lexer class extensions.
 * 
 * This file's function will be referenced, directly or otherwise, by every
 * other function in the parser. The efficiency of its implementation is of
 * crucial importance. If this file runs slow, so do the others.
 * 
 * @section License
 * 
 * Copyright (C) 2011-2014 Josh Ventura
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

#include "lex_cpp.h"
#include <General/debug_macros.h>
#include <General/parse_basics.h>
#include <General/debug_macros.h>
#include <System/builtins.h>
#include <Parser/context_parser.h>
#include <API/context.h>
#include <API/AST.h>
#include <cstring>
#include <csignal>
#include <filesystem>

#include <API/compile_settings.h>

using namespace jdi;
using namespace std;

/// Returns whether s1 begins with s2, followed by whitespace.
// static inline bool strbw(const char* s1, const char (&s2)[3]) { return *s1 == *s2 and s1[1] == s2[1] and (!is_letterd(s1[2])); }
// static inline bool strbw(const char* s1, const char (&s2)[4]) { return *s1 == *s2 and s1[1] == s2[1] and s1[2] == s2[2] and (!is_letterd(s1[3])); }
// static inline bool strbw(const char* s1, const char (&s2)[5]) { return *s1 == *s2 and s1[1] == s2[1] and s1[2] == s2[2] and s1[3] == s2[3] and (!is_letterd(s1[4])); }
// static inline bool strbw(const char* s1, const char (&s2)[6]) { return *s1 == *s2 and s1[1] == s2[1] and s1[2] == s2[2] and s1[3] == s2[3] and s1[4] == s2[4] and (!is_letterd(s1[5])); }
// static inline bool strbw(const char* s1, const char (&s2)[7]) { return *s1 == *s2 and s1[1] == s2[1] and s1[2] == s2[2] and s1[3] == s2[3] and s1[4] == s2[4] and s1[5] == s2[5] and (!is_letterd(s1[6])); }
// static inline bool strbw(const char* s1, const char (&s2)[11]){ return *s1 == *s2 and s1[1] == s2[1] and s1[2] == s2[2] and s1[3] == s2[3] and s1[4] == s2[4] and s1[5] == s2[5] and s1[6] == s2[6] and s1[7] == s2[7] and s1[8] == s2[8] and s1[9] == s2[9] and (!is_letterd(s1[10])); }
static inline bool strbw(char s) { return !is_letterd(s); }

static inline void skip_comment(llreader &cfile) {
  if (ALLOW_MULTILINE_COMMENTS) {
    while (cfile.advance() && cfile.at() != '\n' && cfile.at() != '\r')
      if (cfile.at() == '\\')
        if (cfile.next() == '\n' || cfile.at() == '\r')
          cfile.take_newline();
  } else {
    while (cfile.advance() && cfile.at() != '\n' and cfile.at() != '\r');
  }
}

static inline void skip_multiline_comment(llreader &cfile) {
  cfile.pos += 2; // Skip two chars so we don't break on /*/
  do {
    if (cfile.eof()) return;
    if (cfile.at() == '\n' or cfile.at() == '\r') cfile.take_newline();
  } while (cfile.getc() != '*' or cfile.at() != '/');
  cfile.advance();
}

// Skips an integer-suffix (u, ul, ull, l, lu, ll, llu)
static inline void skip_integer_suffix(llreader &cfile) {
  if (cfile.eof()) return;
  if (cfile.at() == 'u' || cfile.at() == 'U') {
    if (!cfile.advance()) return;
    if (cfile.at() == 'l' || cfile.at() == 'L') {
      const char l = cfile.at();
      if (cfile.next() == l) cfile.advance();
    }
    return;
  }
  // Per ISO, LLu is fine, llU is fine, but Llu and lul are not.
  const char l = cfile.at();
  if (l != 'l' && l != 'L') return;
  if (!cfile.advance()) return;
  if (cfile.at() == l)
    if (!cfile.advance()) return;
  if (cfile.at() == 'u' || cfile.at() == 'U') cfile.advance();
  return;
}

struct StringPrefixFlags {
  bool valid, raw;
  enum : short {
    UNSPECIFIED = 0,
    CHAR8  = 1,  // UTF-8 char.
    CHAR16 = 2,
    WCHAR  = 3,  // Varies whimsically between 2 and 4.
    CHAR32 = 4,
  } width;
};

static inline StringPrefixFlags parse_string_prefix(string_view pre) {
  StringPrefixFlags res;
  // Valid prefixes: R, u8, u8R, u, uR, U, UR, L, or LR.
  size_t i = 0;
  res.raw = false;
  res.valid = false;
  res.width = StringPrefixFlags::UNSPECIFIED;
  // The u, u8, U, and L flags are mutually-exclusive.
  if (pre[i] == 'u') {
    if (++i < pre.length() && pre[i] == '8') {
      ++i;
      res.width = StringPrefixFlags::CHAR8;
    } else {
      res.width = StringPrefixFlags::CHAR16;
    }
  } else if (pre[i] == 'U') {
    res.width = StringPrefixFlags::CHAR32;
    ++i;
  } else if (pre[i] == 'L') {
    res.width = StringPrefixFlags::WCHAR;
    ++i;
  }
  // R may follow any of the above, or stand alone.
  if (i < pre.length() && pre[i] == 'R') {
    res.raw = true;
    ++i;
  }
  res.valid = i == pre.length();
  return res;
}

#if 0
void lexer::skip_string(error_handler *herr)
{
  register const char endc = cfile.at();
  while (cfile.advance() and cfile.at() != endc)
  {
    if (cfile.at() == '\\') {
      if (cfile.next() == '\n') ++line, lpos = pos;
      else if (cfile.at() == '\r') {
        if (cfile.next() != '\n') --pos;
        ++line, lpos = pos; 
      }
    }
    else if (cfile.at() == '\n' or cfile.at() == '\r') {
      herr->error(cfile, "Unterminated string literal");
      break;
    }
  }
  if (cfile.at() != endc)
    herr->error(cfile, "Unterminated string literal");
}

static inline void skip_string(const char* cfile, size_t &pos, size_t length)
{
  register const char endc = cfile.at();
  while (cfile.advance() and cfile.at() != endc)
    if (cfile.at() == '\\') if (cfile[pos++] == '\r' and cfile.at() == '\n') ++pos;
}

void lexer_base::skip_whitespace()
{
  while (!cfile.eof()) {
    while (is_spacer(cfile.at())) if (++pos >= length) return;
    if (cfile.at() == '\n' or (cfile.at() == '\r' and cfile[pos+1] != '\n')) { ++line; lpos = pos++; continue; }
    if (cfile.at() == '/') {
      if (cfile.next() == '/') { skip_comment(); continue; }
      if (cfile.at() == '*') { skip_multiline_comment(); continue; }
      return;
    }
    return;
  }
}
#endif

void lexer::enter_macro(const macro_type &macro) {
  if (macro.value.empty()) return;
  open_macros.emplace_back(macro.name, &macro.value);
}

static inline void lex_error(error_handler *herr, const llreader &cfile, string_view msg) {
  herr->error(msg, cfile.name, cfile.lnum, cfile.pos - cfile.lpos);
}

static inline bool skip_string(llreader &cfile, char qc, error_handler *herr) {
  while (cfile.next() != EOF && cfile.at() != qc) {
    if (cfile.at() == '\\') {
      if (cfile.next() == EOF) {
        lex_error(herr, cfile, "You can't escape the file ending, jackwagon.");
        return false;
      } else if (cfile.at() == '\n') {
        ++cfile.lnum, cfile.lpos = cfile.pos;
      } else if (cfile.at() == '\r') {
        if (cfile.next() != '\n') --cfile.pos;
        ++cfile.lnum, cfile.lpos = cfile.pos; 
      }
    }
    else if (cfile.at() == '\n' or cfile.at() == '\r') {
      lex_error(herr, cfile, "Unterminated string literal");
      return false;
    }
  }
  if (cfile.eof() or cfile.at() != qc) {
    lex_error(herr, cfile, "Unterminated string literal");
    return false;
  }
  return true;
}

/// Invoked while the reader is at the opening quote.
static inline bool skip_rstring(llreader &cfile, error_handler *herr) {
  // Read delimeter
  bool warned = false;
  const size_t spos = cfile.tell();
  while (cfile.next() != EOF && cfile.at() != '(') {
    if (cfile.at() == '\\' || cfile.at() == ' ') {
      if (!warned) {
        herr->warning(cfile, "ISO C++ forbids backslash, space, and quote "
                             "characters in raw-literal delimiters.");
        warned = true;
      }
    }
  }
  const string delim = ")"s + string{cfile.slice(spos + 1)};
  if (!cfile.skip(delim.length())) return false;
  while ((cfile.at() != '"'
        || cfile.slice(cfile.tell() - delim.length(), cfile.tell() - 1) != delim)
        && cfile.advance());
  return !cfile.eof();
}

bool lexer::parse_macro_params(const macro_type &mf, vector<token_vector> *out) {
  cfile.skip_whitespace();
  
  if (cfile.at() != '(') return false;
  cfile.advance();

  vector<token_vector> res;
  res.reserve(mf.args.size());
  
  // Read the parameters into our argument vector
  int too_many_args = 0;
  for (int nestcnt = 1;;) {
    token_t tok = read_token(cfile, herr);
    if (cfile.eof() or cfile.at() != ')') {
      lex_error(herr, cfile, "Unterminated parameters to macro function");
      return false;
    }
    if (tok.type == TT_LEFTPARENTH) ++nestcnt;
    if (tok.type == TT_RIGHTPARENTH) {
      if (!--nestcnt) break;
    }
    if (res.empty()) res.emplace_back();
    if (tok.type == TT_COMMA && nestcnt == 1) {
      if (res.size() < mf.args.size()) {
        res.emplace_back();
        continue;
      } else if (!mf.is_variadic) {
        ++too_many_args;
      }
    }
    
  }
  if (too_many_args) {
    herr->error(cfile, "Too many arguments to macro function `%s`; expected %s but got %s", 
                mf.name, mf.args.size(), mf.args.size() + too_many_args);
  }
  out->swap(res);
  return true;
}

bool lexer::parse_macro_function(const macro_type &mf) {
  bool already_open = false; // Test if we're in this macro already
  vector<openfile>::iterator it = files.begin();
  for (size_t i = 0; i < open_macros.size(); ++i, --it) {
    if (it->name != mf.name) continue;
    already_open = true;
    break;
  }
  if (already_open)
    return true;
  
  size_t spos = cfile.pos, slpos = cfile.lpos, sline = cfile.lnum;
  cfile.skip_whitespace(); // Move to the next "token"
  if (cfile.eof() or cfile.at() != '(') {
    cfile.pos = spos, cfile.lpos = slpos, cfile.lnum = sline;
    return false;
  }
  
  vector<vector<token_t>> params;
  if (!parse_macro_params(mf, &params)) return false;
  vector<token_t> tokens = mf.substitute_and_unroll(params);
  open_macros.emplace_back(mf.name, std::move(tokens));
  return true;
}

string lexer::read_preprocessor_args() {
  for (;;) {
    while (cfile.at() == ' ' or cfile.at() == '\t') if (!cfile.advance()) return "";
    if (cfile.at() == '/') {
      if (cfile.next() == '/') { skip_comment(cfile); return ""; }
      if (cfile.at() == '*') { skip_multiline_comment(cfile); continue; }
      break;
    }
    if (cfile.at_newline()) return "";
    if (cfile.at() == '\\') {
      if (!cfile.advance()) return "";
      cfile.take_newline();
    }
    break;
  }
  string res;
  res.reserve(256);
  size_t spos = cfile.pos;
  while (!cfile.eof() && cfile.at() != '\n' && cfile.at() != '\r') {
    if (cfile.at() == '/') {
      if (cfile.next() == '/') {
        res += cfile.slice(spos, cfile.pos - 1);
        skip_comment(cfile);
        return res;
      }
      if (cfile.at() == '*') {
        res += cfile.slice(spos, cfile.pos - 1);
        res += " ";
        skip_multiline_comment(cfile);
        spos = cfile.pos;
        continue;
      }
    }
    if (cfile.at() == '\'' || cfile.at() == '"') {
      skip_string(cfile, cfile.getc(), herr);
      cfile.advance();
    } else if (cfile.at() == '\\') {
      if (!cfile.advance()) break;
      cfile.take_newline();
    }
    else cfile.advance();
  }
  res += cfile.slice(spos);
  {
    size_t trim = res.length() - 1;
    if (is_useless(res[trim])) {
      while (is_useless(res[--trim]));
      res.erase(++trim);
    }
  }
  return res;
}

#ifdef DEBUG_MODE
/// This function will be passed signals
static void donothing(int) {}
#endif

// Optional AST rendering
#include <General/debug_macros.h>

/**
  @section Implementation
  
  For the preprocessor's implementation, we switch gears a bit. The preprocessor needs
  to be the fastest aspect of the entire parser, as directives are everywhere. As such,
  it operates on a hacked-up switch statement using a whole heap of gotos, designed to
  support a specific set of macros. Inserting a new directive into this switch is still
  simple enough, but unlike other aspects of the parser, is not as trivial as a single
  function call. Don't gripe; originally I was going to use a perfect hash.
**/
void lexer::handle_preprocessor() {
  top:
  bool variadic = false; // Whether this function is variadic
  while ((cfile.at() == ' ' || cfile.at() == '\t') && cfile.advance());
  const size_t pspos = cfile.tell();
  switch (cfile.getc())
  {
    case 'd':
      if (cfile.take("efine") && strbw(cfile.at())) goto case_define;
      goto failout;
    case 'e':
      if (cfile.at() == 'n') {
        if (cfile.take("ndif") && strbw(cfile.at())) goto case_endif;
        goto failout;
      }
      if (cfile.at() == 'l') { 
        if (cfile.next() == 's') {
          if (cfile.next() == 'e') {
            cfile.advance();
            goto case_else;
          }
          goto failout;
        }
        if (cfile.at() == 'i' && cfile.next() == 'f') {
          if (strbw(cfile.next())) goto case_elif;
          if (cfile.take("def") && strbw(cfile.at()))  goto case_elifdef;
          if (cfile.take("ndef") && strbw(cfile.at())) goto case_elifndef;
        }
        goto failout;
      }
      if (cfile.take("rror") && strbw(cfile.at())) goto case_error;
      goto failout;
    case 'i':
      if (cfile.at() == 'f')
      {
        if (strbw(cfile.next())) goto case_if;
        if (cfile.take("def") && strbw(cfile.at()))  goto case_ifdef;
        if (cfile.take("ndef") && strbw(cfile.at())) goto case_ifndef;
        goto failout;
      }
      if (cfile.at() == 'n') {
        cfile.advance();
        if (cfile.take("clude") && strbw(cfile.at()))      goto case_include;
        if (cfile.take("clude_next") && strbw(cfile.at())) goto case_include_next;
        goto failout;
      }
      if (cfile.take("mport") && strbw(cfile.at())) goto case_import;
      goto failout;
    case 'l':
      if (cfile.take("ine") && strbw(cfile.at())) goto case_line;
      goto failout;
    case 'p':
      if (cfile.take("ragma") && strbw(cfile.at())) goto case_pragma;
      goto failout;
    case 'u':
      if (cfile.take("ndef") && strbw(cfile.at())) goto case_undef;
      if (cfile.take("sing") && strbw(cfile.at())) goto case_using;
      goto failout;
    case 'w':
      if (cfile.take("arning") && strbw(cfile.at())) goto case_warning;
      goto failout;
    default: goto failout;
  }
  
  while (false) {  // This is an improvised switch statement; allows "break" to work.
    case_define: {
      string argstrs = read_preprocessor_args();
      const char* argstr = argstrs.c_str();
      if (!conditionals.empty() and !conditionals.back().is_true)
        break;
      size_t i = 0;
      while (is_useless(argstr[i])) ++i;
      if (!is_letter(argstr[i])) {
        herr->error(cfile, "Expected macro definiendum at this point");
      }
      const size_t nsi = i;
      while (is_letterd(argstr[++i]));
      pair<macro_iter, bool> mins = macros.insert(pair<string,macro_type*>(argstrs.substr(nsi,i-nsi),NULL));
      
      if (argstr[i] == '(') {
        vector<string> paramlist;
        while (is_useless(argstr[++i]));
        if (argstr[i] != ')') for (;;) {
          if (!is_letter(argstr[i])) {
            if (argstr[i] == '.' and argstr[i+1] == '.' and argstr[i+2] == '.') {
              variadic = true, i += 3;
              while (is_useless(argstr[i])) ++i;
              if (argstr[i] != ')')
                herr->error(cfile, "Expected end of parameters after variadic");
              break;
            }
            else {
              herr->error(cfile, "Expected parameter name for macro declaration");
              break;
            }
          }
          const size_t si = i;
          while (is_letterd(argstr[++i]));
          paramlist.push_back(argstrs.substr(si, i-si));
          
          while (is_useless(argstr[i])) ++i;
          if (argstr[i] == ')') break;
          if (argstr[i] == ',') { while (is_useless(argstr[++i])); continue; }
          
          // Handle variadic macros (if we are at ...)
          if (argstr[i] == '.' and argstr[i+1] == '.' and argstr[i+2] == '.') {
            i += 2; while (is_useless(argstr[++i]));
            variadic = true;
            if (argstr[i] == ')') break;
            herr->error(cfile, "Expected closing parenthesis at this point; further parameters not allowed following variadic");
          }
          else
            herr->error(cfile, "Expected comma or closing parenthesis at this point");
        }
        
        if (!mins.second) { // If no insertion was made; ie, the macro existed already.
        //  if ((size_t)mins.first->second->argc != paramlist.size())
        //    herr->warning("Redeclaring macro function `" + mins.first->first + '\'', filename, line, pos-lpos);
          mins.first->second = nullptr;
        }
        mins.first->second = std::make_shared<const macro_type>(
            mins.first->first, paramlist, argstrs.substr(++i), variadic, herr);
      } else {
        while (is_useless(argstr[i])) ++i;
        mins.first->second = std::make_shared<const macro_type>(
            mins.first->first,argstrs.substr(i));
      }
    } break;
    case_error: {
        string emsg = read_preprocessor_args();
        if (conditionals.empty() or conditionals.back().is_true)
          herr->error(cfile, "#error " + emsg);
      } break;
      break;
    case_elif:
        if (conditionals.empty())
          herr->error(cfile, "Unexpected #elif directive; no matching #if");
        else {
          if (conditionals.back().is_true)
            conditionals.back().is_true = conditionals.back().can_be_true = false;
          else {
            if (conditionals.back().can_be_true) {
              conditionals.pop_back();
              goto case_if;
            }
          }
        }
      break;
    case_elifdef:
        if (conditionals.empty())
          herr->error(cfile, "Unexpected #elifdef directive; no matching #if");
        else {
          if (conditionals.back().is_true)
            conditionals.back().is_true = conditionals.back().can_be_true = false;
          else {
            if (conditionals.back().can_be_true) {
              conditionals.pop_back();
              goto case_ifdef;
            }
          }
        }
      break;
    case_elifndef:
        if (conditionals.empty())
          herr->error(cfile, "Unexpected #elifndef directive; no matching #if");
        else {
          if (conditionals.back().is_true)
            conditionals.back().is_true = conditionals.back().can_be_true = false;
          else {
            if (conditionals.back().can_be_true) {
              conditionals.pop_back();
              goto case_ifndef;
            }
          }
        }
      break;
    case_else:
        if (conditionals.empty())
          herr->error(cfile, "Unexpected #else directive; no matching #if");
        else {
          if (conditionals.back().is_true)
            conditionals.back().is_true = conditionals.back().can_be_true = false;
          else
            conditionals.back().is_true = conditionals.back().can_be_true;
        }
      break;
    case_endif:
        if (conditionals.empty())
          return
           herr->error(cfile, "Unexpected #endif directive: no open conditionals.");
        conditionals.pop_back();
      break;
    case_if:
        if (conditionals.empty() or conditionals.back().is_true) {
          token_t tok;
          token_vector toks;
          while ((tok = get_token()).type != TT_ENDOFCODE
                              && tok.type != TTM_NEWLINE) {
            toks.push_back(tok);
          }
          lexer l(std::move(toks), herr);
          AST a = parse_expression(&l);
          render_ast(a, "if_directives");
          if (!a.eval({herr, tok}))
            conditionals.push_back(condition(0,1));
          else
            conditionals.push_back(condition(1,0));
        }
        else
          conditionals.push_back(condition(0,0));
      break;
    case_ifdef: {
        cfile.skip_whitespace();
        if (!is_letter(cfile.at())) {
          herr->error(cfile, "Expected identifier to check against macros");
          break;
        }
        const size_t msp = cfile.tell();
        while (is_letterd(cfile.next()));
        string macro(cfile + msp, cfile.tell() - msp);
        if (conditionals.empty() or conditionals.back().is_true) {
          if (macros.find(macro) == macros.end()) {
            token_t res;
            conditionals.push_back(condition(0,1));
            break;
          }
          conditionals.push_back(condition(1,0));
        }
        else
          conditionals.push_back(condition(0,0));
      } break;
    case_ifndef: {
        cfile.skip_whitespace();
        if (!is_letter(cfile.at())) {
          herr->error(cfile, "Expected identifier to check against macros");
          break;
        }
        const size_t msp = cfile.tell();
        while (is_letterd(cfile.next()));
        string macro(cfile+msp, cfile.tell()-msp);
        if (conditionals.empty() or conditionals.back().is_true) {
          if (macros.find(macro) != macros.end()) {
            token_t res;
            conditionals.push_back(condition(0,1));
            break;
          }
          conditionals.push_back(condition(1,0));
        }
        else
          conditionals.push_back(condition(0,0));
      } break;
    case_import:
      break;
    case_include: {
        bool incnext;
        if (true) incnext = false;
        else { case_include_next: incnext = true; }
        
        string fnfind = read_preprocessor_args();
        if (!conditionals.empty() and !conditionals.back().is_true)
      break;
        
        bool chklocal = false;
        char match = '>';
        if (!incnext and fnfind[0] == '"')
          chklocal = true, match = '"';
        else if (fnfind[0] != '<') {
          herr->error(cfile, "Expected filename inside <> or \"\" delimiters");
          break;
        }
        fnfind[0] = '/';
        for (size_t i = 0; i < fnfind.length(); ++i)
          if (fnfind[i] == match) { fnfind.erase(i); break; }
        
        if (files.size() > 9000) {
          herr->error(cfile, "Nested include count is OVER NINE THOUSAAAAAAAAND. Not including another.");
          break;
        }
        
        string incfn, fdir;
        llreader incfile;
        const string path = filesystem::path(cfile.name).parent_path();
        if (chklocal)
          incfile.open((incfn = path + fnfind).c_str());
        for (size_t i = 0; i < builtin->search_dir_count(); ++i) {
          if (incfile.is_open()) break;
          if (!incnext) {
            // cout << "fnfind:" << endl;
            // cout << "  [" << fnfind.length() << "]\"" << fnfind << '"' << endl;
            // cout << "  [" << builtin->search_dir(i).length() << "]\"" << builtin->search_dir(i) << '"' << endl;
            // cout << "  cat: " << flush << (builtin->search_dir(i) + fnfind) << endl;
            incfile.open((incfn = (fdir = builtin->search_dir(i)) + fnfind).c_str());
          }
          else
            incnext = path != builtin->search_dir(i);
        }
        if (!incfile.is_open()) {
          herr->error(cfile, "Could not find " + fnfind.substr(1));
          if (chklocal) cerr << "  Checked " << path << endl;
          for (size_t i = 0; !incfile.is_open() and i < builtin->search_dir_count(); ++i)
            cerr << "  Checked " << builtin->search_dir(i) << endl;
          break;
        }
        
        files.emplace_back(std::move(cfile));
        visited_files.insert(incfn);
        cfile = std::move(incfile);
      } break;
    case_line:
      break;
    case_pragma:
        #ifdef DEBUG_MODE
        {
          string n = read_preprocessor_args();
          if (n == "DEBUG_ENTRY_POINT" and (conditionals.empty() or conditionals.back().is_true)) {
            signal(SIGTRAP, donothing); // Try not to die when we raise hell in the interrupt handler briefly
            asm("INT3;"); // Raise hell in the interrupt handler; the debugger will grab us from here
            cout << "* Debug entry point" << endl;
          }
        }
        #else
          read_preprocessor_args(herr);
        #endif
      break;
    case_undef:
        if (!conditionals.empty() and !conditionals.back().is_true)
          break;
        
        cfile.skip_whitespace();
        if (!is_letter(cfile.at()))
          herr->error(cfile, "Expected macro identifier at this point");
        else {
          const size_t nspos = cfile.tell();
          while (is_letterd(cfile.next()));
          macros.erase((string) cfile.slice(nspos));  // TODO(C++20): remove cast
        }
      break;
    case_using:
      break;
    case_warning: {
        string wmsg = read_preprocessor_args();
        if (conditionals.empty() or conditionals.back().is_true)
          herr->warning(cfile, "#warning " + wmsg);
      } break;
  }
  if (conditionals.empty() or conditionals.back().is_true)
    return;
  
  // skip_to_macro:
  while (!cfile.eof()) {
    if (cfile.at() == '/' || is_useless(cfile.at()))
      cfile.skip_whitespace();
    else if (cfile.at() == '#') {
      cfile.advance();
      goto top;
    }
  }
  herr->error(cfile, "Expected closing preprocessors before end of code");
  return;
  
  failout:
    while (is_letterd(cfile.at()) && cfile.advance());
    herr->error(cfile, "Invalid preprocessor directive `%s'", cfile.slice(pspos));
    if (!cfile.eof())
      while (cfile.at() != '\n' && cfile.at() != '\r' && cfile.advance());
}

token_t jdi::read_token(llreader &cfile, error_handler *herr) {
  #ifdef DEBUG_MODE
    static int number_of_times_GDB_has_dropped_its_ass = 0;
    ++number_of_times_GDB_has_dropped_its_ass;
  #endif
  
  // Dear C++ committee: do you know what would be exponentially more awesome
  // than this line? Just declaring a normal fucking function, please and thanks
  auto mktok = [&cfile](TOKEN_TYPE tp, size_t pos, int length) -> token_t {
    return token_t(tp, cfile.name.c_str(), cfile.lnum, pos - cfile.lpos,
                   cfile + pos, length);
  };
  
  for (;;) {  // Loop until we find something or hit world's end
    if (cfile.eof()) return mktok(TT_ENDOFCODE, cfile.tell(), 0);
    
    // Skip all whitespace
    while (is_useless(cfile.at())) {
      if (cfile.at() == '\n' || cfile.at() == '\r') {
        cfile.take_newline();
        return mktok(TTM_NEWLINE, cfile.tell(), 0);
      }
      if (!cfile.advance()) return mktok(TT_ENDOFCODE, cfile.tell(), 0);
    }
    
    //==============================================================================================
    //====: Check for and handle comments. :========================================================
    //==============================================================================================
    
    const size_t spos = cfile.tell();
    switch (cfile.getc()) {
    // Skip all whitespace
    
    case '/': {
      if (cfile.at() == '*') { skip_multiline_comment(cfile); continue; }
      if (cfile.at() == '/') { skip_comment(cfile); continue; }
      if (cfile.at() == '=') 
        return mktok(TT_DIVIDE_ASSIGN, ++cfile.pos - 2, 2);
      return mktok(TT_SLASH, cfile.pos - 1, 1);
    }
    
    default:
    //==============================================================================================
    //====: Not at a comment. See if we're at an identifier. :======================================
    //==============================================================================================
    
    if (is_letter(cfile[spos])) {  // Check if we're at an identifier or keyword.
      while (!cfile.eof() && is_letterd(cfile.at())) cfile.advance();
      if (cfile.tell() - spos <= 2 && (cfile.at() == '\'' || cfile.at() == '"')) {
        auto prefix = parse_string_prefix(cfile.slice(spos));
        if (prefix.valid) {
          if (prefix.raw) {
            if (skip_rstring(cfile, herr)) cfile.advance();
            return mktok(TT_STRINGLITERAL, spos, cfile.tell() - spos);
          }
          if (skip_string(cfile, cfile.at(), herr)) cfile.advance();
          return mktok(TT_CHARLITERAL, spos, cfile.tell() - spos);
        }
      }
      return mktok(TT_IDENTIFIER, spos, cfile.tell() - spos);
    }
    
    goto unknown;
    
    //==============================================================================================
    //====: Not at an identifier. Maybe at a number? :==============================================
    //==============================================================================================
    
    case '0': { // Check if the number is hexadecimal, binary, or octal.
      // TODO: Handle apostrophes.
      if (cfile.at() == 'x' || cfile.at() == 'X') { // Check if the number is hexadecimal.
        // Here, it is hexadecimal.
        while (cfile.advance() && is_hexdigit(cfile.at()));
        skip_integer_suffix(cfile);
        return mktok(TT_HEXLITERAL, spos, cfile.tell() - spos);
      }
      if (cfile.at() == 'b' || cfile.at() == 'B') { // Check if the number is Binary.
        // In this case, it's binary
        while (cfile.advance() && is_hexdigit(cfile.at()));
        skip_integer_suffix(cfile);
        return mktok(TT_BINLITERAL, spos, cfile.tell() - spos);
      }
      // Turns out, it's octal.
      if (cfile.eof() || !is_octdigit(cfile.at())) {
        // Literal 0. According to ISO, this is octal, because a decimal literal
        // does not start with zero, while octal literals begin with 0 and
        // consist of octal digits.
        return mktok(TT_OCTLITERAL, spos, 1);
      }
      while (cfile.advance() && is_octdigit(cfile.at()));
      skip_integer_suffix(cfile);
      return mktok(TT_OCTLITERAL, spos, cfile.tell() - spos);
    }
    
    case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8':
    case '9': {
      // Turns out, it's decimal.
      handle_decimal:
      while (!cfile.eof() and is_digit(cfile.at())) cfile.advance();
      if (cfile.at() == '.')
        while (cfile.advance() and is_digit(cfile.at()));
      if (cfile.at() == 'e' or cfile.at() == 'E') { // Accept exponents
        if (cfile.next() == '-') cfile.advance();
        if (cfile.eof()) herr->error(cfile, "Numeric literal truncated and end of file.");
        else while (is_digit(cfile.at()) && cfile.advance());
      }
      skip_integer_suffix(cfile);
      return mktok(TT_DECLITERAL, spos, cfile.tell()  -spos);
    }
    
    //==============================================================================================
    //====: Not at a number. Find out where we are. :===============================================
    //==============================================================================================
    
      case ';':
        return mktok(TT_SEMICOLON, spos, 1);
      case ',':
        return mktok(TT_COMMA, spos, 1);
      case '+':
        if (cfile.at() == '+') {
          cfile.advance();
          return mktok(TT_INCREMENT, spos, 2);
        }
        if (cfile.at() == '=') {
          cfile.advance();
          return mktok(TT_ADD_ASSIGN, spos, 2);
        }
        return mktok(TT_ADD_ASSIGN, spos, 1);
      case '-':
        if (cfile.at() == '+') {
          cfile.advance();
          return mktok(TT_INCREMENT, spos, 2);
        }
        if (cfile.at() == '=') {
          cfile.advance();
          return mktok(TT_ADD_ASSIGN, spos, 2);
        }
        if (cfile.at() == '>') {
          cfile.advance();
          return mktok(TT_ARROW, spos, 2);
        }
        return mktok(TT_ADD_ASSIGN, spos, 1);
      case '=':
        if (cfile.at() == cfile[spos]) {
          cfile.advance();
          return mktok(TT_EQUAL_TO, spos, 2);
        }
        return mktok(TT_EQUAL, spos, 1);
      case '&':
        if (cfile.at() == '&') {
          cfile.advance();
          return mktok(TT_AMPERSANDS, spos, 2);
        }
        if (cfile.at() == '=') {
          cfile.advance();
          return mktok(TT_AND_ASSIGN, spos, 2);
        }
        return mktok(TT_AMPERSAND, spos, 1);
      case '|':
        if (cfile.at() == '|') {
          cfile.advance();
          return mktok(TT_PIPES, spos, 2);
        }
        if (cfile.at() == '=') {
          cfile.advance();
          return mktok(TT_OR_ASSIGN, spos, 2);
        }
        return mktok(TT_PIPE, spos, 1);
      case '~':
        if (cfile.at() == '=') {
          cfile.advance();
          return mktok(TT_NEGATE_ASSIGN, spos, 2);
        }
        return mktok(TT_TILDE, spos, 1);
      case '!':
        if (cfile.at() == '=') {
          cfile.advance();
          return mktok(TT_NOT_EQUAL_TO, spos, 2);
        }
        return mktok(TT_NOT, spos, 1);
      case '%':
        if (cfile.at() == '=') {
          cfile.advance();
          return mktok(TT_MODULO_ASSIGN, spos, 2);
        }
        return mktok(TT_MODULO, spos, 1);
      case '*':
        if (cfile.at() == '=') {
          cfile.advance();
          return mktok(TT_MULTIPLY_ASSIGN, spos, 2);
        }
        return mktok(TT_STAR, spos, 1);
      case '^':
        if (cfile.at() == '=') {
          cfile.advance();
          return mktok(TT_XOR_ASSIGN, spos, 2);
        }
        return mktok(TT_NOT, spos, 1);
      case '>':
        if (cfile.at() == '>') {
          cfile.advance();
          return mktok(TT_RSHIFT, spos, 2);
        }
        if (cfile.at() == '=') {
          cfile.advance();
          return mktok(TT_GREATER_EQUAL, spos, 2);
        }
        return mktok(TT_GREATERTHAN, spos, 1);
      case '<':
        if (cfile.at() == '<') {
          cfile.advance();
          return mktok(TT_LSHIFT, spos, 2);
        }
        if (cfile.at() == '=') {
          cfile.advance();
          return mktok(TT_LESS_EQUAL, spos, 2);
        }
        return mktok(TT_LESSTHAN, spos, 1);
      case ':':
        if (cfile.at() == cfile[spos]) {
          cfile.advance();
          return mktok(TT_SCOPE, spos, 2);
        }
        return mktok(TT_COLON, spos, 1);
      case '?':
        return mktok(TT_QUESTIONMARK, spos, 1);
      
      case '.':
          if (is_digit(cfile.at()))
            goto handle_decimal;
          else if (cfile.at() == '.') {
            if (cfile.peek_next() == '.') {
              cfile.skip(2);
              return mktok(TT_ELLIPSIS, spos, 3);
            }
          }
          if (cfile.at() == '*') {
            cfile.advance();
            return mktok(TT_DOT_STAR, spos, 1);
          }
        return mktok(TT_DOT, spos, 1);
      
      case '(': return mktok(TT_LEFTPARENTH,  spos, 1);
      case '[': return mktok(TT_LEFTBRACKET,  spos, 1);
      case '{': return mktok(TT_LEFTBRACE,    spos, 1);
      case '}': return mktok(TT_RIGHTBRACE,   spos, 1);
      case ']': return mktok(TT_RIGHTBRACKET, spos, 1);
      case ')': return mktok(TT_RIGHTPARENTH, spos, 1);
      
      case '#':
        if (cfile.at() == '#') {
          cfile.advance();
          return mktok(TTM_CONCAT, spos, 2);
        }
        return mktok(TTM_TOSTRING, spos, 1);
      
      case '\\':
        // ISO Translation phase 2
        if (cfile.at() != '\n' || cfile.at() != '\r')
          cfile.take_newline();
        continue;
      
      case '"': {
        skip_string(cfile, '"', herr);
        return mktok(TT_STRINGLITERAL, spos, cfile.tell() - spos);
      }
      
      case '\'': {
        skip_string(cfile, '\'', herr);
        return mktok(TT_CHARLITERAL, spos, cfile.tell() - spos);
      }
      
      unknown: {
        char errbuf[320];
        sprintf(errbuf, "Unrecognized symbol (char)0x%02X '%c'", (int)cfile[spos], cfile[spos]);
        herr->error(errbuf);
        return mktok(TT_INVALID, spos, 1);
      }
    }
  }
  
  herr->error(cfile, "UNREACHABLE BLOCK REACHED");
  return mktok(TT_INVALID, cfile.tell(), 0);
}

token_t lexer::get_token() {
  token_t res;
  top:
  do res = read_raw(); while (res.type == TTM_NEWLINE);
  if (res.type == TT_IDENTIFIER) {
    string fn = res.content.toString();
    macro_iter mi;
    
    mi = macros.find(fn);
    if (mi != macros.end()) {
      if (mi->second->is_function) {
        if (parse_macro_function(*mi->second)) {
          // Upon success, restart routine. On failure, treat as identifier.
          goto top;
        }
      } else {
        bool already_open = false; // Test if we're in this macro already
        auto it = files.rbegin();
        for (unsigned i = 0; i < open_macros.size(); ++i) {
          if (it->name == fn) {
            already_open = true;
            break;
          } else {
            ++it;
          }
        }
        if (!already_open) {
          enter_macro(*mi->second);
          goto top;
        }
      }
    }
    
    keyword_map::iterator kwit = keywords.find(fn);
    if (kwit != keywords.end()) {
      if (kwit->second == TT_INVALID) {
        mi = kludge_map.find(fn);
        #ifdef DEBUG_MODE
        if (mi == kludge_map.end())
          cerr << "SYSTEM ERROR! KEYWORD `" << fn << "' IS DEFINED AS INVALID" << endl;
        #endif
        if (mi->second->is_function) {
          if (!parse_macro_function(*mi->second))
            return res;
        } else {
          enter_macro(*mi->second);
        }
        goto top;
      }
      res.type = kwit->second;
      return res;
    }
    
    tf_iter tfit = builtin_declarators.find(fn);
    if (tfit != builtin_declarators.end()) {
      if (tfit->second->usage  & UF_PRIMITIVE)
        res.type = TT_DECLARATOR, res.def = tfit->second->def;
      else
        res.type = TT_DECFLAG, res.def = (definition*)tfit->second;
      return res;
    }
  }
  else if (res.type == TTM_CONCAT) {
    res.report_error(herr, "Extraneous # ignored");
    goto pp_anyway;
  }
  else if (res.type == TTM_TOSTRING) {
    pp_anyway:
    handle_preprocessor();
    goto top;
  }
  else if (res.type == TT_ENDOFCODE) {
    if (pop_file()) {
      return token_t(TT_ENDOFCODE, cfile.name.c_str(), cfile.lnum,
                     cfile.tell() - cfile.lpos, "", 0);
    }
    goto top;
  }
  
  return res;
}

#if 0
token_t lexer_macro::get_token(error_handler *herr)
{
  top:
  token_t res = lex_base->read_raw();
  if (res.type == TTM_NEWLINE) {
    res.type = TT_ENDOFCODE;
    return res;
  }
  if (res.type == TT_IDENTIFIER) {  // Identifiers and keywords stop here.
    static const char zero[] = "0", one[] = "1";
    // magic number: strlen("defined") == 7; this is unlikely to change
    if (res.content.len == 7 && !strncmp((const char*) res.content.str, "defined", 7))
    {
      lex_base->skip_whitespace();
      bool endpar = lex_base->data[lex_base->pos] == '(';
      if (endpar) {
        ++lex_base->pos;
        lex_base->skip_whitespace();
      }
      
      if (!is_letter(lex_base->data[lex_base->pos]))
        herr->error("Expected identifier to look up as macro", lex_base->filename, lex_base->line, lex_base->pos - lex_base->lpos);
      
      const size_t spos = lex_base->pos;
      while (is_letterd(lex_base->data[++lex_base->pos]));
      string macro(lex_base->data + spos, lex_base->pos - spos);
      
      if (endpar) {
        lex_base->skip_whitespace();
        if (lex_base->data[lex_base->pos] != ')')
          herr->error("Expected ending parenthesis for defined()", lex_base->filename, lex_base->line, lex_base->pos-lex_base->lpos);
        ++lex_base->pos;
      }
      
      return mktok(TT_DECLITERAL,lex_base->filename,lex_base->line,lex_base->pos-lex_base->lpos, lex_base->macros.find(macro)==lex_base->macros.end()? zero : one, 1);
    }
    
    const string mn = res.content.toString();
    macro_iter mi = lex_base->macros.find(mn);
    if (mi != lex_base->macros.end()) {
      if (mi->second->argc < 0) {
        bool already_open = false; // Test if we're in this macro already
        quick::stack<openfile>::iterator it = lex_base->files.begin();
        for (unsigned i = 0; i < lex_base->open_macro_count; ++i)
          if (it->filename == mn) { already_open = true; break; }
          else --it;
        if (!already_open) {
          lex_base->enter_macro( mi->second);
          goto top;
        }
      }
      else {
        if (lex_base->parse_macro_function((macro_function*)mi->second, herr))
          goto top;
      }
    }
    
    // This is an undefined identifier; if it were a macro, it would have been expanded by the base lexer.
    // Return token for literal false: zero.
    return mktok(TT_DECLITERAL,lex_base->filename,lex_base->line,lex_base->pos-lex_base->lpos, zero, 1);
  }
  else if (res.type == TT_ENDOFCODE) {
    if (lex_base->pop_file())
      return lex_base->read_raw(herr);  // TT_ENDOFCODE
    goto top;
  }
  
  return res;
}
#endif

bool lexer::pop_file() {
  /*
  Thus, this lexer implementation has four layers of token source data:
    1. The open file stack. Files or string buffers (managed by an llreader) are
       lexed for raw tokens.
    2. Macros used within a file are expanded into tokens, and these buffers of
       tokens are stacked. Per ISO, a macro may not appear twice in this stack.
    3. Rewind operations produce queues of tokens. Each queue is stacked.
    4. During normal lexing operations, minor lookahead may be required.
       Tokens read during lookahead are stacked. */
  if (lookahead_buffer) {
    herr->error(lookahead_buffer->front(),
                "Attempting to pop a buffer while there are lexed tokens "
                "remaining to be returned.");
  }
  
  if (files.empty())
    return true;
  
  // Close whatever file we have open now
  cfile.close();
  
  // Fetch data from top item and pop stack
  openfile &of = files.back();
  cfile.consume(of.file);
  files.pop_back();
  
  return false;
}

macro_map lexer::kludge_map;
lexer::keyword_map lexer::keywords;

lexer::lexer(llreader &input, macro_map &pmacros, error_handler *err):
    cfile(std::move(input)), herr(err) , macros(pmacros){
  if (keywords.empty()) {
    keywords["asm"] = TT_ASM;
    keywords["__asm"] = TT_ASM;
    keywords["__asm__"] = TT_ASM;
    keywords["class"] = TT_CLASS;
    keywords["decltype"] = TT_DECLTYPE;
    keywords["typeid"] = TT_TYPEID;
    keywords["enum"] = TT_ENUM;
    keywords["extern"] = TT_EXTERN;
    keywords["namespace"] = TT_NAMESPACE;
    keywords["operator"] = TT_OPERATORKW;
    keywords["private"] = TT_PRIVATE;
    keywords["protected"] = TT_PROTECTED;
    keywords["public"] = TT_PUBLIC;
    keywords["friend"] = TT_FRIEND;
    keywords["sizeof"] = TT_SIZEOF;
    keywords["__is_empty"] = TT_ISEMPTY;
    keywords["__is_pod"] = TT_ISEMPTY; // FIXME: yeah, this is a hack
    keywords["struct"] = TT_STRUCT;
    keywords["template"] = TT_TEMPLATE;
    keywords["typedef"] = TT_TYPEDEF;
    keywords["typename"] = TT_TYPENAME;
    keywords["union"] = TT_UNION;
    keywords["using"] = TT_USING;
    keywords["new"] = TT_NEW;
    keywords["delete"] = TT_DELETE;
    
    keywords["const_cast"] = TT_CONST_CAST;
    keywords["static_cast"] = TT_STATIC_CAST;
    keywords["dynamic_cast"] = TT_DYNAMIC_CAST;
    keywords["reinterpret_cast"] = TT_REINTERPRET_CAST;
    
    keywords["auto"] = TT_AUTO;
    keywords["alignas"] = TT_ALIGNAS;
    keywords["alignof"] = TT_ALIGNOF;
    keywords["constexpr"] = TT_CONSTEXPR;
    keywords["noexcept"] = TT_NOEXCEPT;
    keywords["static_assert"] = TT_STATIC_ASSERT;
    
    // GNU Extensions
    keywords["__attribute__"] = TT_INVALID;
    keywords["__extension__"] = TT_INVALID;
    keywords["__typeof__"] = TT_INVALID;
    keywords["__typeof"] = TT_INVALID;
   // keywords["__restrict"] = TT_INVALID;
    
    // MinGW Fuckery
    keywords["__MINGW_IMPORT"] = TT_INVALID;
    
    // C++ Extensions
    keywords["false"] = TT_INVALID;
    keywords["true"] = TT_INVALID;
    
    string x(1,'x');
    kludge_map.clear();
    context::global_macros().swap(kludge_map);
    builtin->add_macro_func("__attribute__", x, string(), false);
    builtin->add_macro_func("__typeof__", x, string("int"), false);
    builtin->add_macro_func("__typeof", x, string("int"), false);
    builtin->add_macro("__extension__", string());
    builtin->add_macro("__MINGW_IMPORT", string());
    builtin->add_macro("false", string(1,'0'));
    builtin->add_macro("true", string(1,'1'));
    context::global_macros().swap(kludge_map);
  }
}

lexer::~lexer() {}

void lexer::cleanup() {
  keywords.clear();
  kludge_map.clear();
}

#undef cfile

lexer::condition::condition(bool t, bool cbt): is_true(t), can_be_true(cbt) {}
