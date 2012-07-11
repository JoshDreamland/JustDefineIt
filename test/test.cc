#define __attribute__(x)
#define __extension__
#define true 1
#define false 0

/*
template<typename b> int
#pragma DEBUG_ENTRY_POINT
a(b c) { this should be ignored }*/

/* /
#include <GL/gl.h>
#include <GL/glu.h>
#include <ncurses.h>
#include <X11/Xlib.h>
#include <AL/al.h>
#include <zlib.h>

#include <stdio.h> //printf, NULL
#include <stdlib.h> //malloc
#include <unistd.h> //usleep
#include <time.h> //clock

#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <float.h>
#include <iso646.h>
#include <math.h>
#include <locale.h>
#include <stdarg.h>
#include <wchar.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <malloc.h>

#include <cpio.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <tar.h>
#include <termios.h>
#include <utime.h>

#include <unistd.h>
#include <stdint.h>

#include <time.h>

#include <fcntl.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <setjmp.h>
#include <inttypes.h>
#include <sys/utsname.h>

// Once affected by lack of '?' operator token
#include <ctype.h>
#include <wctype.h>
#include <limits.h>
#include <assert.h>

// Once affected by lack of support for public/private/protected, and by lack of support for constructors.
#include <pthread.h>

#include <sys/stat.h>
#include <sys/time.h>

#include <fenv.h>
#include <tgmath.h>
#include <complex.h>
/* */

/*// Broken in 9565e24
namespace bbq {
  template<typename p> class q { typedef int c; };
}
template<typename a, typename b> class b {};
typedef b<bbq::q<int>::c, int> c;
*/


/* */


#ifndef _GLIBCXX_STRING
#define _GLIBCXX_STRING	1

#pragma GCC system_header

#include <bits/c++config.h>
#include <bits/stringfwd.h>


#ifndef _CHAR_TRAITS_H
#define _CHAR_TRAITS_H 1

#pragma GCC system_header


#ifndef _STL_ALGOBASE_H
#define _STL_ALGOBASE_H 1

#include <bits/c++config.h>
#include <bits/functexcept.h>


#include <bits/cpp_type_traits.h>


#ifndef _EXT_TYPE_TRAITS
#define _EXT_TYPE_TRAITS 1

#pragma GCC system_header

#include <bits/c++config.h>
#include <bits/cpp_type_traits.h>

namespace __gnu_cxx _GLIBCXX_VISIBILITY(default)
{
_GLIBCXX_BEGIN_NAMESPACE_VERSION

  // Define a nested type if some predicate holds.
  template<bool, typename>
    struct __enable_if 
    { };

  template<typename _Tp>
    struct __enable_if<true, _Tp>
    { typedef _Tp __type; };


  // Conditional expression for types. If true, first, if false, second.
  template<bool _Cond, typename _Iftrue, typename _Iffalse>
    struct __conditional_type
    { typedef _Iftrue __type; };

  template<typename _Iftrue, typename _Iffalse>
    struct __conditional_type<false, _Iftrue, _Iffalse>
    { typedef _Iffalse __type; };


  // Given an integral builtin type, return the corresponding unsigned type.
  template<typename _Tp>
    struct __add_unsigned
    { 
    private:
      #pragma DEBUG_ENTRY_POINT
      typedef __enable_if<std::__is_integer<_Tp>::__value, _Tp> __if_type;
      
    public:
      typedef typename __if_type::__type __type; 
    };

  template<>
    struct __add_unsigned<char>
    { typedef unsigned char __type; };

  template<>
    struct __add_unsigned<signed char>
    { typedef unsigned char __type; };

  template<>
    struct __add_unsigned<short>
    { typedef unsigned short __type; };

  template<>
    struct __add_unsigned<int>
    { typedef unsigned int __type; };

  template<>
    struct __add_unsigned<long>
    { typedef unsigned long __type; };

  template<>
    struct __add_unsigned<long long>
    { typedef unsigned long long __type; };

  // Declare but don't define.
  template<>
    struct __add_unsigned<bool>;

  template<>
    struct __add_unsigned<wchar_t>;


  // Given an integral builtin type, return the corresponding signed type.
  template<typename _Tp>
    struct __remove_unsigned
    { 
    private:
      typedef __enable_if<std::__is_integer<_Tp>::__value, _Tp> __if_type;
      
    public:
      typedef typename __if_type::__type __type; 
    };

  template<>
    struct __remove_unsigned<char>
    { typedef signed char __type; };

  template<>
    struct __remove_unsigned<unsigned char>
    { typedef signed char __type; };

  template<>
    struct __remove_unsigned<unsigned short>
    { typedef short __type; };

  template<>
    struct __remove_unsigned<unsigned int>
    { typedef int __type; };

  template<>
    struct __remove_unsigned<unsigned long>
    { typedef long __type; };

  template<>
    struct __remove_unsigned<unsigned long long>
    { typedef long long __type; };

  // Declare but don't define.
  template<>
    struct __remove_unsigned<bool>;

  template<>
    struct __remove_unsigned<wchar_t>;


  // For use in string and vstring.
  template<typename _Type>
    inline bool
    __is_null_pointer(_Type* __ptr)
    { return __ptr == 0; }

  template<typename _Type>
    inline bool
    __is_null_pointer(_Type)
    { return false; }


  // For complex and cmath
  template<typename _Tp, bool = std::__is_integer<_Tp>::__value>
    struct __promote
    { typedef double __type; };

  // No nested __type member for non-integer non-floating point types,
  // allows this type to be used for SFINAE to constrain overloads in
  // <cmath> and <complex> to only the intended types.
  template<typename _Tp>
    struct __promote<_Tp, false>
    { };

  template<>
    struct __promote<long double>
    { typedef long double __type; };

  template<>
    struct __promote<double>
    { typedef double __type; };

  template<>
    struct __promote<float>
    { typedef float __type; };

  template<typename _Tp, typename _Up,
           typename _Tp2 = typename __promote<_Tp>::__type,
           typename _Up2 = typename __promote<_Up>::__type>
    struct __promote_2
    {
      typedef __typeof__(_Tp2() + _Up2()) __type;
    };

  template<typename _Tp, typename _Up, typename _Vp,
           typename _Tp2 = typename __promote<_Tp>::__type,
           typename _Up2 = typename __promote<_Up>::__type,
           typename _Vp2 = typename __promote<_Vp>::__type>
    struct __promote_3
    {
      typedef __typeof__(_Tp2() + _Up2() + _Vp2()) __type;
    };

  template<typename _Tp, typename _Up, typename _Vp, typename _Wp,
           typename _Tp2 = typename __promote<_Tp>::__type,
           typename _Up2 = typename __promote<_Up>::__type,
           typename _Vp2 = typename __promote<_Vp>::__type,
           typename _Wp2 = typename __promote<_Wp>::__type>
    struct __promote_4
    {
      typedef __typeof__(_Tp2() + _Up2() + _Vp2() + _Wp2()) __type;
    };

_GLIBCXX_END_NAMESPACE_VERSION
} // namespace

#endif 

//#include <ext/type_traits.h>
/*#include <ext/numeric_traits.h>
#include <bits/stl_pair.h>
#include <bits/stl_iterator_base_types.h>
#include <bits/stl_iterator_base_funcs.h>
#include <bits/stl_iterator.h>
#include <bits/concept_check.h>
#include <debug/debug.h>
#include <bits/move.h> // For std::swap and _GLIBCXX_MOVE

namespace std _GLIBCXX_VISIBILITY(default)
{
_GLIBCXX_BEGIN_NAMESPACE_VERSION

  // See http://gcc.gnu.org/ml/libstdc++/2004-08/msg00167.html: in a
  // nutshell, we are partially implementing the resolution of DR 187,
  // when it's safe, i.e., the value_types are equal.
  template<bool _BoolType>
    struct __iter_swap
    {
      template<typename _ForwardIterator1, typename _ForwardIterator2>
        static void
        iter_swap(_ForwardIterator1 __a, _ForwardIterator2 __b)
        {
          typedef typename iterator_traits<_ForwardIterator1>::value_type
            _ValueType1;
          _ValueType1 __tmp = _GLIBCXX_MOVE(*__a);
          *__a = _GLIBCXX_MOVE(*__b);
          *__b = _GLIBCXX_MOVE(__tmp);
	}
    };

  template<>
    struct __iter_swap<true>
    {
      template<typename _ForwardIterator1, typename _ForwardIterator2>
        static void 
        iter_swap(_ForwardIterator1 __a, _ForwardIterator2 __b)
        {
          swap(*__a, *__b);
        }
    };

  
  template<typename _ForwardIterator1, typename _ForwardIterator2>
    inline void
    iter_swap(_ForwardIterator1 __a, _ForwardIterator2 __b)
    {
      typedef typename iterator_traits<_ForwardIterator1>::value_type
	_ValueType1;
      typedef typename iterator_traits<_ForwardIterator2>::value_type
	_ValueType2;

      // concept requirements
      __glibcxx_function_requires(_Mutable_ForwardIteratorConcept<
				  _ForwardIterator1>)
      __glibcxx_function_requires(_Mutable_ForwardIteratorConcept<
				  _ForwardIterator2>)
      __glibcxx_function_requires(_ConvertibleConcept<_ValueType1,
				  _ValueType2>)
      __glibcxx_function_requires(_ConvertibleConcept<_ValueType2,
				  _ValueType1>)

      typedef typename iterator_traits<_ForwardIterator1>::reference
	_ReferenceType1;
      typedef typename iterator_traits<_ForwardIterator2>::reference
	_ReferenceType2;
      std::__iter_swap<__are_same<_ValueType1, _ValueType2>::__value
	&& __are_same<_ValueType1&, _ReferenceType1>::__value
	&& __are_same<_ValueType2&, _ReferenceType2>::__value>::
	iter_swap(__a, __b);
    }

  template<typename _ForwardIterator1, typename _ForwardIterator2>
    _ForwardIterator2
    swap_ranges(_ForwardIterator1 __first1, _ForwardIterator1 __last1,
		_ForwardIterator2 __first2)
    {
      // concept requirements
      __glibcxx_function_requires(_Mutable_ForwardIteratorConcept<
				  _ForwardIterator1>)
      __glibcxx_function_requires(_Mutable_ForwardIteratorConcept<
				  _ForwardIterator2>)
      __glibcxx_requires_valid_range(__first1, __last1);

      for (; __first1 != __last1; ++__first1, ++__first2)
	std::iter_swap(__first1, __first2);
      return __first2;
    }

  template<typename _Tp>
    inline const _Tp&
    min(const _Tp& __a, const _Tp& __b)
    {
      // concept requirements
      __glibcxx_function_requires(_LessThanComparableConcept<_Tp>)
      //return __b < __a ? __b : __a;
      if (__b < __a)
	return __b;
      return __a;
    }

  template<typename _Tp>
    inline const _Tp&
    max(const _Tp& __a, const _Tp& __b)
    {
      // concept requirements
      __glibcxx_function_requires(_LessThanComparableConcept<_Tp>)
      //return  __a < __b ? __b : __a;
      if (__a < __b)
	return __b;
      return __a;
    }

  template<typename _Tp, typename _Compare>
    inline const _Tp&
    min(const _Tp& __a, const _Tp& __b, _Compare __comp)
    {
      //return __comp(__b, __a) ? __b : __a;
      if (__comp(__b, __a))
	return __b;
      return __a;
    }

  template<typename _Tp, typename _Compare>
    inline const _Tp&
    max(const _Tp& __a, const _Tp& __b, _Compare __comp)
    {
      //return __comp(__a, __b) ? __b : __a;
      if (__comp(__a, __b))
	return __b;
      return __a;
    }

  // If _Iterator is a __normal_iterator return its base (a plain pointer,
  // normally) otherwise return it untouched.  See copy, fill, ... 
  template<typename _Iterator>
    struct _Niter_base
    : _Iter_base<_Iterator, __is_normal_iterator<_Iterator>::__value>
    { };

  template<typename _Iterator>
    inline typename _Niter_base<_Iterator>::iterator_type
    __niter_base(_Iterator __it)
    { return std::_Niter_base<_Iterator>::_S_base(__it); }

  // Likewise, for move_iterator.
  template<typename _Iterator>
    struct _Miter_base
    : _Iter_base<_Iterator, __is_move_iterator<_Iterator>::__value>
    { };

  template<typename _Iterator>
    inline typename _Miter_base<_Iterator>::iterator_type
    __miter_base(_Iterator __it)
    { return std::_Miter_base<_Iterator>::_S_base(__it); }

  // All of these auxiliary structs serve two purposes.  (1) Replace
  // calls to copy with memmove whenever possible.  (Memmove, not memcpy,
  // because the input and output ranges are permitted to overlap.)
  // (2) If we're using random access iterators, then write the loop as
  // a for loop with an explicit count.

  template<bool, bool, typename>
    struct __copy_move
    {
      template<typename _II, typename _OI>
        static _OI
        __copy_m(_II __first, _II __last, _OI __result)
        {
	  for (; __first != __last; ++__result, ++__first)
	    *__result = *__first;
	  return __result;
	}
    };

#ifdef __GXX_EXPERIMENTAL_CXX0X__
  template<typename _Category>
    struct __copy_move<true, false, _Category>
    {
      template<typename _II, typename _OI>
        static _OI
        __copy_m(_II __first, _II __last, _OI __result)
        {
	  for (; __first != __last; ++__result, ++__first)
	    *__result = std::move(*__first);
	  return __result;
	}
    };
#endif

  template<>
    struct __copy_move<false, false, random_access_iterator_tag>
    {
      template<typename _II, typename _OI>
        static _OI
        __copy_m(_II __first, _II __last, _OI __result)
        { 
	  typedef typename iterator_traits<_II>::difference_type _Distance;
	  for(_Distance __n = __last - __first; __n > 0; --__n)
	    {
	      *__result = *__first;
	      ++__first;
	      ++__result;
	    }
	  return __result;
	}
    };

#ifdef __GXX_EXPERIMENTAL_CXX0X__
  template<>
    struct __copy_move<true, false, random_access_iterator_tag>
    {
      template<typename _II, typename _OI>
        static _OI
        __copy_m(_II __first, _II __last, _OI __result)
        { 
	  typedef typename iterator_traits<_II>::difference_type _Distance;
	  for(_Distance __n = __last - __first; __n > 0; --__n)
	    {
	      *__result = std::move(*__first);
	      ++__first;
	      ++__result;
	    }
	  return __result;
	}
    };
#endif

  template<bool _IsMove>
    struct __copy_move<_IsMove, true, random_access_iterator_tag>
    {
      template<typename _Tp>
        static _Tp*
        __copy_m(const _Tp* __first, const _Tp* __last, _Tp* __result)
        {
	  const ptrdiff_t _Num = __last - __first;
	  if (_Num)
	    __builtin_memmove(__result, __first, sizeof(_Tp) * _Num);
	  return __result + _Num;
	}
    };

  template<bool _IsMove, typename _II, typename _OI>
    inline _OI
    __copy_move_a(_II __first, _II __last, _OI __result)
    {
      typedef typename iterator_traits<_II>::value_type _ValueTypeI;
      typedef typename iterator_traits<_OI>::value_type _ValueTypeO;
      typedef typename iterator_traits<_II>::iterator_category _Category;
      const bool __simple = (__is_trivial(_ValueTypeI)
	                     && __is_pointer<_II>::__value
	                     && __is_pointer<_OI>::__value
			     && __are_same<_ValueTypeI, _ValueTypeO>::__value);

      return std::__copy_move<_IsMove, __simple,
	                      _Category>::__copy_m(__first, __last, __result);
    }

  // Helpers for streambuf iterators (either istream or ostream).
  // NB: avoid including <iosfwd>, relatively large.
  template<typename _CharT>
    struct char_traits;

  template<typename _CharT, typename _Traits>
    class istreambuf_iterator;

  template<typename _CharT, typename _Traits>
    class ostreambuf_iterator;

  template<bool _IsMove, typename _CharT>
    typename __gnu_cxx::__enable_if<__is_char<_CharT>::__value, 
	     ostreambuf_iterator<_CharT, char_traits<_CharT> > >::__type
    __copy_move_a2(_CharT*, _CharT*,
		   ostreambuf_iterator<_CharT, char_traits<_CharT> >);

  template<bool _IsMove, typename _CharT>
    typename __gnu_cxx::__enable_if<__is_char<_CharT>::__value, 
	     ostreambuf_iterator<_CharT, char_traits<_CharT> > >::__type
    __copy_move_a2(const _CharT*, const _CharT*,
		   ostreambuf_iterator<_CharT, char_traits<_CharT> >);

  template<bool _IsMove, typename _CharT>
    typename __gnu_cxx::__enable_if<__is_char<_CharT>::__value,
				    _CharT*>::__type
    __copy_move_a2(istreambuf_iterator<_CharT, char_traits<_CharT> >,
		   istreambuf_iterator<_CharT, char_traits<_CharT> >, _CharT*);

  template<bool _IsMove, typename _II, typename _OI>
    inline _OI
    __copy_move_a2(_II __first, _II __last, _OI __result)
    {
      return _OI(std::__copy_move_a<_IsMove>(std::__niter_base(__first),
					     std::__niter_base(__last),
					     std::__niter_base(__result)));
    }

  template<typename _II, typename _OI>
    inline _OI
    copy(_II __first, _II __last, _OI __result)
    {
      // concept requirements
      __glibcxx_function_requires(_InputIteratorConcept<_II>)
      __glibcxx_function_requires(_OutputIteratorConcept<_OI,
	    typename iterator_traits<_II>::value_type>)
      __glibcxx_requires_valid_range(__first, __last);

      return (std::__copy_move_a2<__is_move_iterator<_II>::__value>
	      (std::__miter_base(__first), std::__miter_base(__last),
	       __result));
    }

#ifdef __GXX_EXPERIMENTAL_CXX0X__
  template<typename _II, typename _OI>
    inline _OI
    move(_II __first, _II __last, _OI __result)
    {
      // concept requirements
      __glibcxx_function_requires(_InputIteratorConcept<_II>)
      __glibcxx_function_requires(_OutputIteratorConcept<_OI,
	    typename iterator_traits<_II>::value_type>)
      __glibcxx_requires_valid_range(__first, __last);

      return std::__copy_move_a2<true>(std::__miter_base(__first),
				       std::__miter_base(__last), __result);
    }

#define _GLIBCXX_MOVE3(_Tp, _Up, _Vp) std::move(_Tp, _Up, _Vp)
#else
#define _GLIBCXX_MOVE3(_Tp, _Up, _Vp) std::copy(_Tp, _Up, _Vp)
#endif

  template<bool, bool, typename>
    struct __copy_move_backward
    {
      template<typename _BI1, typename _BI2>
        static _BI2
        __copy_move_b(_BI1 __first, _BI1 __last, _BI2 __result)
        {
	  while (__first != __last)
	    *--__result = *--__last;
	  return __result;
	}
    };

#ifdef __GXX_EXPERIMENTAL_CXX0X__
  template<typename _Category>
    struct __copy_move_backward<true, false, _Category>
    {
      template<typename _BI1, typename _BI2>
        static _BI2
        __copy_move_b(_BI1 __first, _BI1 __last, _BI2 __result)
        {
	  while (__first != __last)
	    *--__result = std::move(*--__last);
	  return __result;
	}
    };
#endif

  template<>
    struct __copy_move_backward<false, false, random_access_iterator_tag>
    {
      template<typename _BI1, typename _BI2>
        static _BI2
        __copy_move_b(_BI1 __first, _BI1 __last, _BI2 __result)
        {
	  typename iterator_traits<_BI1>::difference_type __n;
	  for (__n = __last - __first; __n > 0; --__n)
	    *--__result = *--__last;
	  return __result;
	}
    };

#ifdef __GXX_EXPERIMENTAL_CXX0X__
  template<>
    struct __copy_move_backward<true, false, random_access_iterator_tag>
    {
      template<typename _BI1, typename _BI2>
        static _BI2
        __copy_move_b(_BI1 __first, _BI1 __last, _BI2 __result)
        {
	  typename iterator_traits<_BI1>::difference_type __n;
	  for (__n = __last - __first; __n > 0; --__n)
	    *--__result = std::move(*--__last);
	  return __result;
	}
    };
#endif

  template<bool _IsMove>
    struct __copy_move_backward<_IsMove, true, random_access_iterator_tag>
    {
      template<typename _Tp>
        static _Tp*
        __copy_move_b(const _Tp* __first, const _Tp* __last, _Tp* __result)
        {
	  const ptrdiff_t _Num = __last - __first;
	  if (_Num)
	    __builtin_memmove(__result - _Num, __first, sizeof(_Tp) * _Num);
	  return __result - _Num;
	}
    };

  template<bool _IsMove, typename _BI1, typename _BI2>
    inline _BI2
    __copy_move_backward_a(_BI1 __first, _BI1 __last, _BI2 __result)
    {
      typedef typename iterator_traits<_BI1>::value_type _ValueType1;
      typedef typename iterator_traits<_BI2>::value_type _ValueType2;
      typedef typename iterator_traits<_BI1>::iterator_category _Category;
      const bool __simple = (__is_trivial(_ValueType1)
	                     && __is_pointer<_BI1>::__value
	                     && __is_pointer<_BI2>::__value
			     && __are_same<_ValueType1, _ValueType2>::__value);

      return std::__copy_move_backward<_IsMove, __simple,
	                               _Category>::__copy_move_b(__first,
								 __last,
								 __result);
    }

  template<bool _IsMove, typename _BI1, typename _BI2>
    inline _BI2
    __copy_move_backward_a2(_BI1 __first, _BI1 __last, _BI2 __result)
    {
      return _BI2(std::__copy_move_backward_a<_IsMove>
		  (std::__niter_base(__first), std::__niter_base(__last),
		   std::__niter_base(__result)));
    }

  template<typename _BI1, typename _BI2>
    inline _BI2
    copy_backward(_BI1 __first, _BI1 __last, _BI2 __result)
    {
      // concept requirements
      __glibcxx_function_requires(_BidirectionalIteratorConcept<_BI1>)
      __glibcxx_function_requires(_Mutable_BidirectionalIteratorConcept<_BI2>)
      __glibcxx_function_requires(_ConvertibleConcept<
	    typename iterator_traits<_BI1>::value_type,
	    typename iterator_traits<_BI2>::value_type>)
      __glibcxx_requires_valid_range(__first, __last);

      return (std::__copy_move_backward_a2<__is_move_iterator<_BI1>::__value>
	      (std::__miter_base(__first), std::__miter_base(__last),
	       __result));
    }

#ifdef __GXX_EXPERIMENTAL_CXX0X__
  template<typename _BI1, typename _BI2>
    inline _BI2
    move_backward(_BI1 __first, _BI1 __last, _BI2 __result)
    {
      // concept requirements
      __glibcxx_function_requires(_BidirectionalIteratorConcept<_BI1>)
      __glibcxx_function_requires(_Mutable_BidirectionalIteratorConcept<_BI2>)
      __glibcxx_function_requires(_ConvertibleConcept<
	    typename iterator_traits<_BI1>::value_type,
	    typename iterator_traits<_BI2>::value_type>)
      __glibcxx_requires_valid_range(__first, __last);

      return std::__copy_move_backward_a2<true>(std::__miter_base(__first),
						std::__miter_base(__last),
						__result);
    }

#define _GLIBCXX_MOVE_BACKWARD3(_Tp, _Up, _Vp) std::move_backward(_Tp, _Up, _Vp)
#else
#define _GLIBCXX_MOVE_BACKWARD3(_Tp, _Up, _Vp) std::copy_backward(_Tp, _Up, _Vp)
#endif

  template<typename _ForwardIterator, typename _Tp>
    inline typename
    __gnu_cxx::__enable_if<!__is_scalar<_Tp>::__value, void>::__type
    __fill_a(_ForwardIterator __first, _ForwardIterator __last,
 	     const _Tp& __value)
    {
      for (; __first != __last; ++__first)
	*__first = __value;
    }
    
  template<typename _ForwardIterator, typename _Tp>
    inline typename
    __gnu_cxx::__enable_if<__is_scalar<_Tp>::__value, void>::__type
    __fill_a(_ForwardIterator __first, _ForwardIterator __last,
	     const _Tp& __value)
    {
      const _Tp __tmp = __value;
      for (; __first != __last; ++__first)
	*__first = __tmp;
    }

  // Specialization: for char types we can use memset.
  template<typename _Tp>
    inline typename
    __gnu_cxx::__enable_if<__is_byte<_Tp>::__value, void>::__type
    __fill_a(_Tp* __first, _Tp* __last, const _Tp& __c)
    {
      const _Tp __tmp = __c;
      __builtin_memset(__first, static_cast<unsigned char>(__tmp),
		       __last - __first);
    }

  template<typename _ForwardIterator, typename _Tp>
    inline void
    fill(_ForwardIterator __first, _ForwardIterator __last, const _Tp& __value)
    {
      // concept requirements
      __glibcxx_function_requires(_Mutable_ForwardIteratorConcept<
				  _ForwardIterator>)
      __glibcxx_requires_valid_range(__first, __last);

      std::__fill_a(std::__niter_base(__first), std::__niter_base(__last),
		    __value);
    }

  template<typename _OutputIterator, typename _Size, typename _Tp>
    inline typename
    __gnu_cxx::__enable_if<!__is_scalar<_Tp>::__value, _OutputIterator>::__type
    __fill_n_a(_OutputIterator __first, _Size __n, const _Tp& __value)
    {
      for (__decltype(__n + 0) __niter = __n;
	   __niter > 0; --__niter, ++__first)
	*__first = __value;
      return __first;
    }

  template<typename _OutputIterator, typename _Size, typename _Tp>
    inline typename
    __gnu_cxx::__enable_if<__is_scalar<_Tp>::__value, _OutputIterator>::__type
    __fill_n_a(_OutputIterator __first, _Size __n, const _Tp& __value)
    {
      const _Tp __tmp = __value;
      for (__decltype(__n + 0) __niter = __n;
	   __niter > 0; --__niter, ++__first)
	*__first = __tmp;
      return __first;
    }

  template<typename _Size, typename _Tp>
    inline typename
    __gnu_cxx::__enable_if<__is_byte<_Tp>::__value, _Tp*>::__type
    __fill_n_a(_Tp* __first, _Size __n, const _Tp& __c)
    {
      std::__fill_a(__first, __first + __n, __c);
      return __first + __n;
    }

  template<typename _OI, typename _Size, typename _Tp>
    inline _OI
    fill_n(_OI __first, _Size __n, const _Tp& __value)
    {
      // concept requirements
      __glibcxx_function_requires(_OutputIteratorConcept<_OI, _Tp>)

      return _OI(std::__fill_n_a(std::__niter_base(__first), __n, __value));
    }

  template<bool _BoolType>
    struct __equal
    {
      template<typename _II1, typename _II2>
        static bool
        equal(_II1 __first1, _II1 __last1, _II2 __first2)
        {
	  for (; __first1 != __last1; ++__first1, ++__first2)
	    if (!(*__first1 == *__first2))
	      return false;
	  return true;
	}
    };

  template<>
    struct __equal<true>
    {
      template<typename _Tp>
        static bool
        equal(const _Tp* __first1, const _Tp* __last1, const _Tp* __first2)
        {
	  return !__builtin_memcmp(__first1, __first2, sizeof(_Tp)
				   * (__last1 - __first1));
	}
    };

  template<typename _II1, typename _II2>
    inline bool
    __equal_aux(_II1 __first1, _II1 __last1, _II2 __first2)
    {
      typedef typename iterator_traits<_II1>::value_type _ValueType1;
      typedef typename iterator_traits<_II2>::value_type _ValueType2;
      const bool __simple = (__is_integer<_ValueType1>::__value
	                     && __is_pointer<_II1>::__value
	                     && __is_pointer<_II2>::__value
			     && __are_same<_ValueType1, _ValueType2>::__value);

      return std::__equal<__simple>::equal(__first1, __last1, __first2);
    }


  template<typename, typename>
    struct __lc_rai
    {
      template<typename _II1, typename _II2>
        static _II1
        __newlast1(_II1, _II1 __last1, _II2, _II2)
        { return __last1; }

      template<typename _II>
        static bool
        __cnd2(_II __first, _II __last)
        { return __first != __last; }
    };

  template<>
    struct __lc_rai<random_access_iterator_tag, random_access_iterator_tag>
    {
      template<typename _RAI1, typename _RAI2>
        static _RAI1
        __newlast1(_RAI1 __first1, _RAI1 __last1,
		   _RAI2 __first2, _RAI2 __last2)
        {
	  const typename iterator_traits<_RAI1>::difference_type
	    __diff1 = __last1 - __first1;
	  const typename iterator_traits<_RAI2>::difference_type
	    __diff2 = __last2 - __first2;
	  return __diff2 < __diff1 ? __first1 + __diff2 : __last1;
	}

      template<typename _RAI>
        static bool
        __cnd2(_RAI, _RAI)
        { return true; }
    };

  template<bool _BoolType>
    struct __lexicographical_compare
    {
      template<typename _II1, typename _II2>
        static bool __lc(_II1, _II1, _II2, _II2);
    };

  template<bool _BoolType>
    template<typename _II1, typename _II2>
      bool
      __lexicographical_compare<_BoolType>::
      __lc(_II1 __first1, _II1 __last1, _II2 __first2, _II2 __last2)
      {
	typedef typename iterator_traits<_II1>::iterator_category _Category1;
	typedef typename iterator_traits<_II2>::iterator_category _Category2;
	typedef std::__lc_rai<_Category1, _Category2> 	__rai_type;
	
	__last1 = __rai_type::__newlast1(__first1, __last1,
					 __first2, __last2);
	for (; __first1 != __last1 && __rai_type::__cnd2(__first2, __last2);
	     ++__first1, ++__first2)
	  {
	    if (*__first1 < *__first2)
	      return true;
	    if (*__first2 < *__first1)
	      return false;
	  }
	return __first1 == __last1 && __first2 != __last2;
      }

  template<>
    struct __lexicographical_compare<true>
    {
      template<typename _Tp, typename _Up>
        static bool
        __lc(const _Tp* __first1, const _Tp* __last1,
	     const _Up* __first2, const _Up* __last2)
	{
	  const size_t __len1 = __last1 - __first1;
	  const size_t __len2 = __last2 - __first2;
	  const int __result = __builtin_memcmp(__first1, __first2,
						std::min(__len1, __len2));
	  return __result != 0 ? __result < 0 : __len1 < __len2;
	}
    };

  template<typename _II1, typename _II2>
    inline bool
    __lexicographical_compare_aux(_II1 __first1, _II1 __last1,
				  _II2 __first2, _II2 __last2)
    {
      typedef typename iterator_traits<_II1>::value_type _ValueType1;
      typedef typename iterator_traits<_II2>::value_type _ValueType2;
      const bool __simple =
	(__is_byte<_ValueType1>::__value && __is_byte<_ValueType2>::__value
	 && !__gnu_cxx::__numeric_traits<_ValueType1>::__is_signed
	 && !__gnu_cxx::__numeric_traits<_ValueType2>::__is_signed
	 && __is_pointer<_II1>::__value
	 && __is_pointer<_II2>::__value);

      return std::__lexicographical_compare<__simple>::__lc(__first1, __last1,
							    __first2, __last2);
    }

  template<typename _ForwardIterator, typename _Tp>
    _ForwardIterator
    lower_bound(_ForwardIterator __first, _ForwardIterator __last,
		const _Tp& __val)
    {
      typedef typename iterator_traits<_ForwardIterator>::value_type
	_ValueType;
      typedef typename iterator_traits<_ForwardIterator>::difference_type
	_DistanceType;

      // concept requirements
      __glibcxx_function_requires(_ForwardIteratorConcept<_ForwardIterator>)
      __glibcxx_function_requires(_LessThanOpConcept<_ValueType, _Tp>)
      __glibcxx_requires_partitioned_lower(__first, __last, __val);

      _DistanceType __len = std::distance(__first, __last);

      while (__len > 0)
	{
	  _DistanceType __half = __len >> 1;
	  _ForwardIterator __middle = __first;
	  std::advance(__middle, __half);
	  if (*__middle < __val)
	    {
	      __first = __middle;
	      ++__first;
	      __len = __len - __half - 1;
	    }
	  else
	    __len = __half;
	}
      return __first;
    }

  /// This is a helper function for the sort routines and for random.tcc.
  //  Precondition: __n > 0.
  template<typename _Size>
    inline _Size
    __lg(_Size __n)
    {
      _Size __k;
      for (__k = 0; __n != 0; __n >>= 1)
	++__k;
      return __k - 1;
    }

  inline int
  __lg(int __n)
  { return sizeof(int) * __CHAR_BIT__  - 1 - __builtin_clz(__n); }

  inline long
  __lg(long __n)
  { return sizeof(long) * __CHAR_BIT__ - 1 - __builtin_clzl(__n); }

  inline long long
  __lg(long long __n)
  { return sizeof(long long) * __CHAR_BIT__ - 1 - __builtin_clzll(__n); }

_GLIBCXX_END_NAMESPACE_VERSION

_GLIBCXX_BEGIN_NAMESPACE_ALGO

  template<typename _II1, typename _II2>
    inline bool
    equal(_II1 __first1, _II1 __last1, _II2 __first2)
    {
      // concept requirements
      __glibcxx_function_requires(_InputIteratorConcept<_II1>)
      __glibcxx_function_requires(_InputIteratorConcept<_II2>)
      __glibcxx_function_requires(_EqualOpConcept<
	    typename iterator_traits<_II1>::value_type,
	    typename iterator_traits<_II2>::value_type>)
      __glibcxx_requires_valid_range(__first1, __last1);

      return std::__equal_aux(std::__niter_base(__first1),
			      std::__niter_base(__last1),
			      std::__niter_base(__first2));
    }

  template<typename _IIter1, typename _IIter2, typename _BinaryPredicate>
    inline bool
    equal(_IIter1 __first1, _IIter1 __last1,
	  _IIter2 __first2, _BinaryPredicate __binary_pred)
    {
      // concept requirements
      __glibcxx_function_requires(_InputIteratorConcept<_IIter1>)
      __glibcxx_function_requires(_InputIteratorConcept<_IIter2>)
      __glibcxx_requires_valid_range(__first1, __last1);

      for (; __first1 != __last1; ++__first1, ++__first2)
	if (!bool(__binary_pred(*__first1, *__first2)))
	  return false;
      return true;
    }

  template<typename _II1, typename _II2>
    inline bool
    lexicographical_compare(_II1 __first1, _II1 __last1,
			    _II2 __first2, _II2 __last2)
    {
      // concept requirements
      typedef typename iterator_traits<_II1>::value_type _ValueType1;
      typedef typename iterator_traits<_II2>::value_type _ValueType2;
      __glibcxx_function_requires(_InputIteratorConcept<_II1>)
      __glibcxx_function_requires(_InputIteratorConcept<_II2>)
      __glibcxx_function_requires(_LessThanOpConcept<_ValueType1, _ValueType2>)
      __glibcxx_function_requires(_LessThanOpConcept<_ValueType2, _ValueType1>)
      __glibcxx_requires_valid_range(__first1, __last1);
      __glibcxx_requires_valid_range(__first2, __last2);

      return std::__lexicographical_compare_aux(std::__niter_base(__first1),
						std::__niter_base(__last1),
						std::__niter_base(__first2),
						std::__niter_base(__last2));
    }

  template<typename _II1, typename _II2, typename _Compare>
    bool
    lexicographical_compare(_II1 __first1, _II1 __last1,
			    _II2 __first2, _II2 __last2, _Compare __comp)
    {
      typedef typename iterator_traits<_II1>::iterator_category _Category1;
      typedef typename iterator_traits<_II2>::iterator_category _Category2;
      typedef std::__lc_rai<_Category1, _Category2> 	__rai_type;

      // concept requirements
      __glibcxx_function_requires(_InputIteratorConcept<_II1>)
      __glibcxx_function_requires(_InputIteratorConcept<_II2>)
      __glibcxx_requires_valid_range(__first1, __last1);
      __glibcxx_requires_valid_range(__first2, __last2);

      __last1 = __rai_type::__newlast1(__first1, __last1, __first2, __last2);
      for (; __first1 != __last1 && __rai_type::__cnd2(__first2, __last2);
	   ++__first1, ++__first2)
	{
	  if (__comp(*__first1, *__first2))
	    return true;
	  if (__comp(*__first2, *__first1))
	    return false;
	}
      return __first1 == __last1 && __first2 != __last2;
    }

  template<typename _InputIterator1, typename _InputIterator2>
    pair<_InputIterator1, _InputIterator2>
    mismatch(_InputIterator1 __first1, _InputIterator1 __last1,
	     _InputIterator2 __first2)
    {
      // concept requirements
      __glibcxx_function_requires(_InputIteratorConcept<_InputIterator1>)
      __glibcxx_function_requires(_InputIteratorConcept<_InputIterator2>)
      __glibcxx_function_requires(_EqualOpConcept<
	    typename iterator_traits<_InputIterator1>::value_type,
	    typename iterator_traits<_InputIterator2>::value_type>)
      __glibcxx_requires_valid_range(__first1, __last1);

      while (__first1 != __last1 && *__first1 == *__first2)
        {
	  ++__first1;
	  ++__first2;
        }
      return pair<_InputIterator1, _InputIterator2>(__first1, __first2);
    }

  template<typename _InputIterator1, typename _InputIterator2,
	   typename _BinaryPredicate>
    pair<_InputIterator1, _InputIterator2>
    mismatch(_InputIterator1 __first1, _InputIterator1 __last1,
	     _InputIterator2 __first2, _BinaryPredicate __binary_pred)
    {
      // concept requirements
      __glibcxx_function_requires(_InputIteratorConcept<_InputIterator1>)
      __glibcxx_function_requires(_InputIteratorConcept<_InputIterator2>)
      __glibcxx_requires_valid_range(__first1, __last1);

      while (__first1 != __last1 && bool(__binary_pred(*__first1, *__first2)))
        {
	  ++__first1;
	  ++__first2;
        }
      return pair<_InputIterator1, _InputIterator2>(__first1, __first2);
    }

_GLIBCXX_END_NAMESPACE_ALGO
} // namespace std

// NB: This file is included within many other C++ includes, as a way
// of getting the base algorithms. So, make sure that parallel bits
// come in too if requested. 
#ifdef _GLIBCXX_PARALLEL
# include <parallel/algobase.h>
#endif

#endif


//#include <bits/stl_algobase.h>  // std::copy, std::fill_n

/*#include <bits/postypes.h>      // For streampos
#include <cwchar>               // For WEOF, wmemmove, wmemset, etc.

namespace __gnu_cxx _GLIBCXX_VISIBILITY(default)
{
_GLIBCXX_BEGIN_NAMESPACE_VERSION

  template<typename _CharT>
    struct _Char_types
    {
      typedef unsigned long   int_type;
      typedef std::streampos  pos_type;
      typedef std::streamoff  off_type;
      typedef std::mbstate_t  state_type;
    };


  
  template<typename _CharT>
    struct char_traits
    {
      typedef _CharT                                    char_type;
      typedef typename _Char_types<_CharT>::int_type    int_type;
      typedef typename _Char_types<_CharT>::pos_type    pos_type;
      typedef typename _Char_types<_CharT>::off_type    off_type;
      typedef typename _Char_types<_CharT>::state_type  state_type;

      static void
      assign(char_type& __c1, const char_type& __c2)
      { __c1 = __c2; }

      static _GLIBCXX_CONSTEXPR bool
      eq(const char_type& __c1, const char_type& __c2)
      { return __c1 == __c2; }

      static _GLIBCXX_CONSTEXPR bool
      lt(const char_type& __c1, const char_type& __c2)
      { return __c1 < __c2; }

      static int
      compare(const char_type* __s1, const char_type* __s2, std::size_t __n);

      static std::size_t
      length(const char_type* __s);

      static const char_type*
      find(const char_type* __s, std::size_t __n, const char_type& __a);

      static char_type*
      move(char_type* __s1, const char_type* __s2, std::size_t __n);

      static char_type*
      copy(char_type* __s1, const char_type* __s2, std::size_t __n);

      static char_type*
      assign(char_type* __s, std::size_t __n, char_type __a);

      static _GLIBCXX_CONSTEXPR char_type
      to_char_type(const int_type& __c)
      { return static_cast<char_type>(__c); }

      static _GLIBCXX_CONSTEXPR int_type
      to_int_type(const char_type& __c)
      { return static_cast<int_type>(__c); }

      static _GLIBCXX_CONSTEXPR bool
      eq_int_type(const int_type& __c1, const int_type& __c2)
      { return __c1 == __c2; }

      static _GLIBCXX_CONSTEXPR int_type
      eof()
      { return static_cast<int_type>(_GLIBCXX_STDIO_EOF); }

      static _GLIBCXX_CONSTEXPR int_type
      not_eof(const int_type& __c)
      { return !eq_int_type(__c, eof()) ? __c : to_int_type(char_type()); }
    };

  template<typename _CharT>
    int
    char_traits<_CharT>::
    compare(const char_type* __s1, const char_type* __s2, std::size_t __n)
    {
      for (std::size_t __i = 0; __i < __n; ++__i)
	if (lt(__s1[__i], __s2[__i]))
	  return -1;
	else if (lt(__s2[__i], __s1[__i]))
	  return 1;
      return 0;
    }

  template<typename _CharT>
    std::size_t
    char_traits<_CharT>::
    length(const char_type* __p)
    {
      std::size_t __i = 0;
      while (!eq(__p[__i], char_type()))
        ++__i;
      return __i;
    }

  template<typename _CharT>
    const typename char_traits<_CharT>::char_type*
    char_traits<_CharT>::
    find(const char_type* __s, std::size_t __n, const char_type& __a)
    {
      for (std::size_t __i = 0; __i < __n; ++__i)
        if (eq(__s[__i], __a))
          return __s + __i;
      return 0;
    }

  template<typename _CharT>
    typename char_traits<_CharT>::char_type*
    char_traits<_CharT>::
    move(char_type* __s1, const char_type* __s2, std::size_t __n)
    {
      return static_cast<_CharT*>(__builtin_memmove(__s1, __s2,
						    __n * sizeof(char_type)));
    }

  template<typename _CharT>
    typename char_traits<_CharT>::char_type*
    char_traits<_CharT>::
    copy(char_type* __s1, const char_type* __s2, std::size_t __n)
    {
      // NB: Inline std::copy so no recursive dependencies.
      std::copy(__s2, __s2 + __n, __s1);
      return __s1;
    }

  template<typename _CharT>
    typename char_traits<_CharT>::char_type*
    char_traits<_CharT>::
    assign(char_type* __s, std::size_t __n, char_type __a)
    {
      // NB: Inline std::fill_n so no recursive dependencies.
      std::fill_n(__s, __n, __a);
      return __s;
    }

_GLIBCXX_END_NAMESPACE_VERSION
} // namespace

namespace std _GLIBCXX_VISIBILITY(default)
{
_GLIBCXX_BEGIN_NAMESPACE_VERSION

  // 21.1
  
  template<class _CharT>
    struct char_traits : public __gnu_cxx::char_traits<_CharT>
    { };


  /// 21.1.3.1  char_traits specializations
  template<>
    struct char_traits<char>
    {
      typedef char              char_type;
      typedef int               int_type;
      typedef streampos         pos_type;
      typedef streamoff         off_type;
      typedef mbstate_t         state_type;

      static void
      assign(char_type& __c1, const char_type& __c2)
      { __c1 = __c2; }

      static _GLIBCXX_CONSTEXPR bool
      eq(const char_type& __c1, const char_type& __c2)
      { return __c1 == __c2; }

      static _GLIBCXX_CONSTEXPR bool
      lt(const char_type& __c1, const char_type& __c2)
      { return __c1 < __c2; }

      static int
      compare(const char_type* __s1, const char_type* __s2, size_t __n)
      { return __builtin_memcmp(__s1, __s2, __n); }

      static size_t
      length(const char_type* __s)
      { return __builtin_strlen(__s); }

      static const char_type*
      find(const char_type* __s, size_t __n, const char_type& __a)
      { return static_cast<const char_type*>(__builtin_memchr(__s, __a, __n)); }

      static char_type*
      move(char_type* __s1, const char_type* __s2, size_t __n)
      { return static_cast<char_type*>(__builtin_memmove(__s1, __s2, __n)); }

      static char_type*
      copy(char_type* __s1, const char_type* __s2, size_t __n)
      { return static_cast<char_type*>(__builtin_memcpy(__s1, __s2, __n)); }

      static char_type*
      assign(char_type* __s, size_t __n, char_type __a)
      { return static_cast<char_type*>(__builtin_memset(__s, __a, __n)); }

      static _GLIBCXX_CONSTEXPR char_type
      to_char_type(const int_type& __c)
      { return static_cast<char_type>(__c); }

      // To keep both the byte 0xff and the eof symbol 0xffffffff
      // from ending up as 0xffffffff.
      static _GLIBCXX_CONSTEXPR int_type
      to_int_type(const char_type& __c)
      { return static_cast<int_type>(static_cast<unsigned char>(__c)); }

      static _GLIBCXX_CONSTEXPR bool
      eq_int_type(const int_type& __c1, const int_type& __c2)
      { return __c1 == __c2; }

      static _GLIBCXX_CONSTEXPR int_type
      eof()
      { return static_cast<int_type>(_GLIBCXX_STDIO_EOF); }

      static _GLIBCXX_CONSTEXPR int_type
      not_eof(const int_type& __c)
      { return (__c == eof()) ? 0 : __c; }
  };


#ifdef _GLIBCXX_USE_WCHAR_T
  /// 21.1.3.2  char_traits specializations
  template<>
    struct char_traits<wchar_t>
    {
      typedef wchar_t           char_type;
      typedef wint_t            int_type;
      typedef streamoff         off_type;
      typedef wstreampos        pos_type;
      typedef mbstate_t         state_type;

      static void
      assign(char_type& __c1, const char_type& __c2)
      { __c1 = __c2; }

      static _GLIBCXX_CONSTEXPR bool
      eq(const char_type& __c1, const char_type& __c2)
      { return __c1 == __c2; }

      static _GLIBCXX_CONSTEXPR bool
      lt(const char_type& __c1, const char_type& __c2)
      { return __c1 < __c2; }

      static int
      compare(const char_type* __s1, const char_type* __s2, size_t __n)
      { return wmemcmp(__s1, __s2, __n); }

      static size_t
      length(const char_type* __s)
      { return wcslen(__s); }

      static const char_type*
      find(const char_type* __s, size_t __n, const char_type& __a)
      { return wmemchr(__s, __a, __n); }

      static char_type*
      move(char_type* __s1, const char_type* __s2, size_t __n)
      { return wmemmove(__s1, __s2, __n); }

      static char_type*
      copy(char_type* __s1, const char_type* __s2, size_t __n)
      { return wmemcpy(__s1, __s2, __n); }

      static char_type*
      assign(char_type* __s, size_t __n, char_type __a)
      { return wmemset(__s, __a, __n); }

      static _GLIBCXX_CONSTEXPR char_type
      to_char_type(const int_type& __c)
      { return char_type(__c); }

      static _GLIBCXX_CONSTEXPR int_type
      to_int_type(const char_type& __c)
      { return int_type(__c); }

      static _GLIBCXX_CONSTEXPR bool
      eq_int_type(const int_type& __c1, const int_type& __c2)
      { return __c1 == __c2; }

      static _GLIBCXX_CONSTEXPR int_type
      eof()
      { return static_cast<int_type>(WEOF); }

      static _GLIBCXX_CONSTEXPR int_type
      not_eof(const int_type& __c)
      { return eq_int_type(__c, eof()) ? 0 : __c; }
  };
#endif //_GLIBCXX_USE_WCHAR_T

_GLIBCXX_END_NAMESPACE_VERSION
} // namespace

#if (defined(__GXX_EXPERIMENTAL_CXX0X__) \
     && defined(_GLIBCXX_USE_C99_STDINT_TR1))

#include <cstdint>

namespace std _GLIBCXX_VISIBILITY(default)
{
_GLIBCXX_BEGIN_NAMESPACE_VERSION

  template<>
    struct char_traits<char16_t>
    {
      typedef char16_t          char_type;
      typedef uint_least16_t    int_type;
      typedef streamoff         off_type;
      typedef u16streampos      pos_type;
      typedef mbstate_t         state_type;

      static void
      assign(char_type& __c1, const char_type& __c2)
      { __c1 = __c2; }

      static _GLIBCXX_CONSTEXPR bool
      eq(const char_type& __c1, const char_type& __c2)
      { return __c1 == __c2; }

      static _GLIBCXX_CONSTEXPR bool
      lt(const char_type& __c1, const char_type& __c2)
      { return __c1 < __c2; }

      static int
      compare(const char_type* __s1, const char_type* __s2, size_t __n)
      {
	for (size_t __i = 0; __i < __n; ++__i)
	  if (lt(__s1[__i], __s2[__i]))
	    return -1;
	  else if (lt(__s2[__i], __s1[__i]))
	    return 1;
	return 0;
      }

      static size_t
      length(const char_type* __s)
      {
	size_t __i = 0;
	while (!eq(__s[__i], char_type()))
	  ++__i;
	return __i;
      }

      static const char_type*
      find(const char_type* __s, size_t __n, const char_type& __a)
      {
	for (size_t __i = 0; __i < __n; ++__i)
	  if (eq(__s[__i], __a))
	    return __s + __i;
	return 0;
      }

      static char_type*
      move(char_type* __s1, const char_type* __s2, size_t __n)
      {
	return (static_cast<char_type*>
		(__builtin_memmove(__s1, __s2, __n * sizeof(char_type))));
      }

      static char_type*
      copy(char_type* __s1, const char_type* __s2, size_t __n)
      {
	return (static_cast<char_type*>
		(__builtin_memcpy(__s1, __s2, __n * sizeof(char_type))));
      }

      static char_type*
      assign(char_type* __s, size_t __n, char_type __a)
      {
	for (size_t __i = 0; __i < __n; ++__i)
	  assign(__s[__i], __a);
	return __s;
      }

      static _GLIBCXX_CONSTEXPR char_type
      to_char_type(const int_type& __c)
      { return char_type(__c); }

      static _GLIBCXX_CONSTEXPR int_type
      to_int_type(const char_type& __c)
      { return int_type(__c); }

      static _GLIBCXX_CONSTEXPR bool
      eq_int_type(const int_type& __c1, const int_type& __c2)
      { return __c1 == __c2; }

      static _GLIBCXX_CONSTEXPR int_type
      eof()
      { return static_cast<int_type>(-1); }

      static _GLIBCXX_CONSTEXPR int_type
      not_eof(const int_type& __c)
      { return eq_int_type(__c, eof()) ? 0 : __c; }
    };

  template<>
    struct char_traits<char32_t>
    {
      typedef char32_t          char_type;
      typedef uint_least32_t    int_type;
      typedef streamoff         off_type;
      typedef u32streampos      pos_type;
      typedef mbstate_t         state_type;

      static void
      assign(char_type& __c1, const char_type& __c2)
      { __c1 = __c2; }

      static _GLIBCXX_CONSTEXPR bool
      eq(const char_type& __c1, const char_type& __c2)
      { return __c1 == __c2; }

      static _GLIBCXX_CONSTEXPR bool
      lt(const char_type& __c1, const char_type& __c2)
      { return __c1 < __c2; }

      static int
      compare(const char_type* __s1, const char_type* __s2, size_t __n)
      {
	for (size_t __i = 0; __i < __n; ++__i)
	  if (lt(__s1[__i], __s2[__i]))
	    return -1;
	  else if (lt(__s2[__i], __s1[__i]))
	    return 1;
	return 0;
      }

      static size_t
      length(const char_type* __s)
      {
	size_t __i = 0;
	while (!eq(__s[__i], char_type()))
	  ++__i;
	return __i;
      }

      static const char_type*
      find(const char_type* __s, size_t __n, const char_type& __a)
      {
	for (size_t __i = 0; __i < __n; ++__i)
	  if (eq(__s[__i], __a))
	    return __s + __i;
	return 0;
      }

      static char_type*
      move(char_type* __s1, const char_type* __s2, size_t __n)
      {
	return (static_cast<char_type*>
		(__builtin_memmove(__s1, __s2, __n * sizeof(char_type))));
      }

      static char_type*
      copy(char_type* __s1, const char_type* __s2, size_t __n)
      { 
	return (static_cast<char_type*>
		(__builtin_memcpy(__s1, __s2, __n * sizeof(char_type))));
      }

      static char_type*
      assign(char_type* __s, size_t __n, char_type __a)
      {
	for (size_t __i = 0; __i < __n; ++__i)
	  assign(__s[__i], __a);
	return __s;
      }

      static _GLIBCXX_CONSTEXPR char_type
      to_char_type(const int_type& __c)
      { return char_type(__c); }

      static _GLIBCXX_CONSTEXPR int_type
      to_int_type(const char_type& __c)
      { return int_type(__c); }

      static _GLIBCXX_CONSTEXPR bool
      eq_int_type(const int_type& __c1, const int_type& __c2)
      { return __c1 == __c2; }

      static _GLIBCXX_CONSTEXPR int_type
      eof()
      { return static_cast<int_type>(-1); }

      static _GLIBCXX_CONSTEXPR int_type
      not_eof(const int_type& __c)
      { return eq_int_type(__c, eof()) ? 0 : __c; }
    };

_GLIBCXX_END_NAMESPACE_VERSION
} // namespace

#endif 

#endif // _CHAR_TRAITS_H


//#include <bits/char_traits.h>
/* // NB: In turn includes stl_algobase.h
#include <bits/allocator.h>
#include <bits/cpp_type_traits.h>
#include <bits/localefwd.h>    // For operators >>, <<, and getline.
#include <bits/ostream_insert.h>
#include <bits/stl_iterator_base_types.h>
#include <bits/stl_iterator_base_funcs.h>
#include <bits/stl_iterator.h>
#include <bits/stl_function.h> // For less
#include <ext/numeric_traits.h> 
#include <bits/stl_algobase.h> 
#include <bits/range_access.h>
#include <bits/basic_string.h>
#include <bits/basic_string.tcc> 

#endif /* _GLIBCXX_STRING */

