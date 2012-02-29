#include "full_type.h"

namespace jdi {
  full_type::full_type(): def(NULL) {}
  full_type::full_type(jdi::definition* d): def(d) {}
  full_type::full_type(jdi::definition* d, jdi::ref_stack r, int f): def(d), refs(r), flags(f) {}
  full_type::full_type(const full_type& ft): def(ft.def), refs(ft.refs), flags(ft.flags) { }
}
