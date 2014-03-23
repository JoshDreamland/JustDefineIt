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


#ifndef	_MATH_H
#define	_MATH_H	1

#include <features.h>

#define __MATHCALL(function,suffix, args)	\
  __MATHDECL (_Mdouble_,function,suffix, args)
#define __MATHDECL(type, function,suffix, args) \
  __MATHDECL_1(type, function,suffix, args); \
  __MATHDECL_1(type, __CONCAT(__,function),suffix, args)
#define __MATHCALLX(function,suffix, args, attrib)	\
  __MATHDECLX (_Mdouble_,function,suffix, args, attrib)
#define __MATHDECLX(type, function,suffix, args, attrib) \
  __MATHDECL_1(type, function,suffix, args) __attribute__ (attrib); \
  __MATHDECL_1(type, __CONCAT(__,function),suffix, args) __attribute__ (attrib)
#define __MATHDECL_1(type, function,suffix, args) \
  extern type __MATH_PRECNAME(function,suffix) args __THROW

#define _Mdouble_		double
#define __MATH_PRECNAME(name,r)	__CONCAT(name,r)
#define _Mdouble_BEGIN_NAMESPACE __BEGIN_NAMESPACE_STD
#define _Mdouble_END_NAMESPACE   __END_NAMESPACE_STD


/* Return the fractional part of X after dividing out `ilogb (X)'.  */
__MATHCALL (significand,, (_Mdouble_ __x));
#endif /* Use misc.  */


#if defined __USE_MISC || defined __USE_XOPEN || defined __USE_ISOC99
__BEGIN_NAMESPACE_C99
__MATHCALL (lgamma,, (_Mdouble_));
__END_NAMESPACE_C99
#endif


#ifdef __USE_MISC
/* Reentrant version of lgamma.  This function uses the global variable
   `signgam'.  The reentrant version instead takes a pointer and stores
   the value through it.  */
   #pragma DEBUG_ENTRY_POINT
__MATHCALL (lgamma,_r, (_Mdouble_, int *__signgamp));
#endif




/* * /
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
#include <functional>
#include <algorithm>
#include <memory>
#include <list>
#include <deque>
#include <vector>
#include <stack>
#include <queue>
#include <set>
#include <map>
#include <limits>
#include <string>
#include <stdexcept>
#include <bitset>
/* */

/* * /
#include <iomanip>    // 2
#include <streambuf>  // 13
#include <ios>        // 27
#include <locale>     // 39
#include <ostream>    // 50
#include <iterator>   // 107
#include <istream>    // 107
#include <iostream>   // 107
#include <fstream>    // 113
#include <sstream>    // 121
#include <strstream>  // 129
#include <hash_set>   // 182
#include <complex>    // 265
/* */

