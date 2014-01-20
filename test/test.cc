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
#include <algorithm>
#include <memory>
#include <functional>
/* */

/* * /
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

template<typename a> struct phuk {
  phuk(int) {}
  typedef double q;
  enum { r = 10 };
};

class myclass {
  myclass *a;
  myclass(*b)();
  myclass(int x);
  myclass(myclass  *c): a(0) {}
  myclass(myclass::*c)(myclass);
  myclass(myclass::*)(myclass);
  myclass(phuk<int> X3);
  myclass(phuk<int>::q X3);
  myclass(int(phuk<int>::q)): a(0) {}
  static int bait(phuk<int>::q) { return 0; }
  
  int ign1 = myclass(10);
  int ign2 = myclass(myclass(10));
  int ign3 = myclass(int(phuk<int>::r));
  int ign4 = myclass(bait);
  int ign4 = (this ->* x)(*this);
};

myclass(myclass::*x)(myclass) = c;

int main() {
return 0;
}
