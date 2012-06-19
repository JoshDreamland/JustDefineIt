/// @file  lexer_interface.cpp
/// @brief Microfile to implement the virtual destructor for \c lexer.
#include "lexer_interface.h"
jdi::lexer::~lexer() {}

using namespace jdip;

token_t lexer::get_token_in_scope(jdi::definition_scope *scope, error_handler *herr) {
  token_t res = get_token(herr);
  
  if (res.type == TT_IDENTIFIER) {
    const string name(res.content.toString());
    definition *def = scope->look_up(name);
    if (!def) {
      #ifdef DEBUG_MODE
        res.def = NULL;
      #endif
      return res;
    }
    res.def = def;
    
    if (def->flags & DEF_TYPENAME) {
      res.type = TT_DECLARATOR;
      return res;
    }
    
    res.type = TT_DEFINITION;
  }
  
  return res;
}
