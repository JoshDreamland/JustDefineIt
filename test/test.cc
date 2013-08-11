/*
typedef int x;
template<typename _Tp> struct _Array { _Array(x); };
#pragma DEBUG_ENTRY_POINT
template<typename _Tp> inline _Array<_Tp>::_Array(x __n) { }
*/

/*
template<int x> class b;
template<int x> class
#pragma DEBUG_ENTRY_POINT
b: syntax_error {};
*/

/* Simple C test case * /
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/file.h>
#include <GL/gl.h>
#include <AL/al.h>

/* * /

template<typename t1, typename t2> class my_template {
  t1 my_member;
  typename t2::whatever my_other_member;
};

template<typename t2> class my_template<int, t2> {
  int fantastic;
  typename t2::hahahahahaha hahahaha;
};

my_template<int, long> qool;

enum tits { mcgavin = 10 };

template<int x> class plusone {
  enum val {
    value = x + 1,
    ass = mcgavin
  };
};

class two: plusone<1> {
  enum v {
    x = value
  };
};
/* * /
template<int x> struct factorial {
  enum r {
    value = x * factorial<x - 1>::value
  };
};

template<> struct factorial<1> {
  enum r {
    value = 1
  };
};

class seventwenty: factorial<6> {
};

template<bool e, class t, class f> class ternary {
  typedef void tp;
};

template<class t, class f> class ternary<true, t, f> {
  typedef t tp;
};

template<class t, class f> class ternary<false, t, f> {
  typedef f tp;
};


template<typename o, typename p, typename q> struct matches {
  enum { value = 1 };
};

template<typename qq, typename pp> struct matches<pp, qq, qq> {
  enum { value = 2 };
};

template<typename pp, typename qq> struct matches<pp, pp, qq> {
  enum { value = 2 };
};

template<typename pp, typename qq> struct matches<pp, qq, pp> {
  enum { value = 2 };
};

template<typename pp> struct matches<pp, pp, pp> {
  enum { value = 3 };
};

template<typename t> struct is_pointer {
  enum { value = 0 };
}
template<typename t> struct is_pointer<t*> {
  enum { value = 1 };
}

template<typename t> struct is_const {
  enum { value = 0 };
}
template<typename t> struct is_const<const t> {
  enum { value = 1 };
}

template<typename t> struct is_int { enum { value = false }; };
template<> struct is_int<int> { enum { value = true }; };

/* */

/* * /
struct redherring {};
namespace c {
  template<typename t> t red_herring(t x) { return 0; }
}

/* * /

struct string {};

template<typename t> t add_one(t x) { return (++x)--; }
string add_one(string x) { return x + "1"; }
template<> string add_one(string x) { return x + "1"; }

/* */

#include <cmath>
#include <cstdio>
#include <cstdlib>

/*
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cfloat>
#include <ciso646>
#include <climits>
#include <clocale>
#include <cmath>
#include <csetjmp>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cwchar>
#include <cwctype>
#include <csignal>

#include <iso646.h>

#include <exception>
#include <utility>
#include <new>
#include <numeric>
*/
/*
#include <algorithm>
#include <bitset>
#include <complex>
#include <deque>
#include <fstream>
#include <functional>
#include <hash_map>
#include <hash_set>
#include <iomanip>
#include <ios>
#include <iosfwd>
#include <iostream>
#include <istream>
#include <iterator>
#include <limits>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <ostream>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <strstream>
#include <vector>
*/


