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

template<int x> class what {
  int a;
  what(
       #pragma DEBUG_ENTRY_POINT
       what &__a):
    a(x)
    {}
  template<typename q> what(q hel) {}
};
