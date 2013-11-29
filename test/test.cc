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
#include <zlib.h>
#include <png.h>

/* */

// In progress
//#include <bits/stl_algobase.h>  // std::copy, std::fill_n

// Down the road
// #include <bits/char_traits.h>
/*
// Way down the road
#include <string>

// The north pole
#include <iostream>

/* */
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
//#include <algorithm>
/* */
/* * /
#include <memory>
#include <functional>
#include <list>
#include <deque>
#include <vector>
#include <stack>
#include <queue>
#include <set>
#include <map>
#include <string>
#include <stdexcept>
#include <bitset>
#include <complex>
#include <fstream>
#include <hash_map>
#include <hash_set>
#include <iomanip>
#include <ios>
#include <iosfwd>
#include <iostream>
#include <istream>
#include <iterator>
#include <limits>
#include <locale>
#include <ostream>
#include <sstream>
#include <streambuf>
#include <strstream>
/* */

/*
template<typename a> class is_double { enum { isdbl = 0 }; };
template<> class is_double<double>   { enum { isdbl = 1 }; };
template<int seven> class is_seven   { enum { is7   = 0 }; };
template<> class is_seven<7>         { enum { is7   = 1 }; };
template<typename a, int n> class is_d7: public is_double<a>, is_seven<n> {};
typedef double da;
typedef da  da2;
typedef da2 da3;
*/

#include <vector>
/*
enum {
  confusion = (int
  #pragma DEBUG_ENTRY_POINT
  (*)(int, int))(1+1)
};

int a;
class my_class {
  my_class();
  my_class(int x, double d);
  typedef int t;
  t q;
  int f();
};

my_class::t sommat;
int my_class::f() { return 0; }

int f();
int f() {}
int f(int x);
int f(int x) {}

my_class::my_class() {}
my_class::my_class(int x, double d): q(x) {}

my_class::my_class() {}

template<typename a> class my_tclass {
  a x;
  my_tclass(a x);
};
template<typename a>
  my_tclass<a>::my_tclass(a x):
      x(x) {
        return fucknuts;
        zero;
    }

template<typename t> struct arses {
  template<typename q> arses(t x);
};

template<typename t, typename q> arses<q>::arses(t x) {}
*/
