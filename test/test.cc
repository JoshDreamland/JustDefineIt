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
#include <functional>
#include <algorithm>
#include <memory>
/* */

/* */
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

/*
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
*/


//
//struct asses {
//  typedef int squirrel;
//  int lemon(squirrel q);
//};
//int asses::lemon(squirrel q) {};
//
//template<typename wee> class yay {};
//template<class x> class y {
//  typedef typename x::template rebind<yay<int> >::z w;
//};
//
//
//#ifndef _GLIBCXX_LIST
//#define _GLIBCXX_LIST 1
//
//#pragma GCC system_header
//
//#include <bits/stl_algobase.h>
//#include <bits/allocator.h>
//#include <bits/range_access.h>
//#include <bits/stl_list.h>
////#include <bits/list.tcc>
//
//
//
//
//#ifndef _LIST_TCC
//#define _LIST_TCC 1
//
//namespace std _GLIBCXX_VISIBILITY(default)
//{
//_GLIBCXX_BEGIN_NAMESPACE_CONTAINER
//
//  template<typename _Tp, typename _Alloc>
//    void
//    _List_base<_Tp, _Alloc>::
//    _M_clear()
//    {
//      typedef _List_node<_Tp>  _Node;
//      _Node* __cur = static_cast<_Node*>(_M_impl._M_node._M_next);
//      while (__cur != &_M_impl._M_node)
//        {
//          _Node* __tmp = __cur;
//          __cur = static_cast<_Node*>(__cur->_M_next);
//#if __cplusplus >= 201103L
//          _M_get_Node_allocator().destroy(__tmp);
//#else
//          _M_get_Tp_allocator().destroy(std::__addressof(__tmp->_M_data));
//#endif
//          _M_put_node(__tmp);
//        }
//    }
//
//#if __cplusplus >= 201103L
//  template<typename _Tp, typename _Alloc>
//    template<typename... _Args>
//      typename list<_Tp, _Alloc>::iterator
//      list<_Tp, _Alloc>::
//      emplace(iterator __position, _Args&&... __args)
//      {
//        _Node* __tmp = _M_create_node(std::forward<_Args>(__args)...);
//        __tmp->_M_hook(__position._M_node);
//        return iterator(__tmp);
//      }
//#endif
//
//  template<typename _Tp, typename _Alloc>
//    typename list<_Tp, _Alloc>::iterator
//    list<_Tp, _Alloc
//    #pragma DEBUG_ENTRY_POINT
//    >::
//    insert(iterator __position, const value_type& __x)
//    {
//      _Node* __tmp = _M_create_node(__x);
//      __tmp->_M_hook(__position._M_node);
//      return iterator(__tmp);
//    }
//
//  template<typename _Tp, typename _Alloc>
//    typename list<_Tp, _Alloc>::iterator
//    list<_Tp, _Alloc>::
//    erase(iterator __position)
//    {
//      iterator __ret = iterator(__position._M_node->_M_next);
//      _M_erase(__position);
//      return __ret;
//    }
//
//#if __cplusplus >= 201103L
//  template<typename _Tp, typename _Alloc>
//    void
//    list<_Tp, _Alloc>::
//    _M_default_append(size_type __n)
//    {
//      size_type __i = 0;
//      __try
//        {
//          for (; __i < __n; ++__i)
//            emplace_back();
//        }
//      __catch(...)
//        {
//          for (; __i; --__i)
//            pop_back();
//          __throw_exception_again;
//        }
//    }
//
//  template<typename _Tp, typename _Alloc>
//    void
//    list<_Tp, _Alloc>::
//    resize(size_type __new_size)
//    {
//      iterator __i = begin();
//      size_type __len = 0;
//      for (; __i != end() && __len < __new_size; ++__i, ++__len)
//        ;
//      if (__len == __new_size)
//        erase(__i, end());
//      else                          // __i == end()
//        _M_default_append(__new_size - __len);
//    }
//
//  template<typename _Tp, typename _Alloc>
//    void
//    list<_Tp, _Alloc>::
//    resize(size_type __new_size, const value_type& __x)
//    {
//      iterator __i = begin();
//      size_type __len = 0;
//      for (; __i != end() && __len < __new_size; ++__i, ++__len)
//        ;
//      if (__len == __new_size)
//        erase(__i, end());
//      else                          // __i == end()
//        insert(end(), __new_size - __len, __x);
//    }
//#else
//  template<typename _Tp, typename _Alloc>
//    void
//    list<_Tp, _Alloc>::
//    resize(size_type __new_size, value_type __x)
//    {
//      iterator __i = begin();
//      size_type __len = 0;
//      for (; __i != end() && __len < __new_size; ++__i, ++__len)
//        ;
//      if (__len == __new_size)
//        erase(__i, end());
//      else                          // __i == end()
//        insert(end(), __new_size - __len, __x);
//    }
//#endif
//
//  template<typename _Tp, typename _Alloc>
//    list<_Tp, _Alloc>&
//    list<_Tp, _Alloc>::
//    operator=(const list& __x)
//    {
//      if (this != &__x)
//        {
//          iterator __first1 = begin();
//          iterator __last1 = end();
//          const_iterator __first2 = __x.begin();
//          const_iterator __last2 = __x.end();
//          for (; __first1 != __last1 && __first2 != __last2;
//               ++__first1, ++__first2)
//            *__first1 = *__first2;
//          if (__first2 == __last2)
//            erase(__first1, __last1);
//          else
//            insert(__last1, __first2, __last2);
//        }
//      return *this;
//    }
//
//  template<typename _Tp, typename _Alloc>
//    void
//    list<_Tp, _Alloc>::
//    _M_fill_assign(size_type __n, const value_type& __val)
//    {
//      iterator __i = begin();
//      for (; __i != end() && __n > 0; ++__i, --__n)
//        *__i = __val;
//      if (__n > 0)
//        insert(end(), __n, __val);
//      else
//        erase(__i, end());
//    }
//
//  template<typename _Tp, typename _Alloc>
//    template <typename _InputIterator>
//      void
//      list<_Tp, _Alloc>::
//      _M_assign_dispatch(_InputIterator __first2, _InputIterator __last2,
//                         __false_type)
//      {
//        iterator __first1 = begin();
//        iterator __last1 = end();
//        for (; __first1 != __last1 && __first2 != __last2;
//             ++__first1, ++__first2)
//          *__first1 = *__first2;
//        if (__first2 == __last2)
//          erase(__first1, __last1);
//        else
//          insert(__last1, __first2, __last2);
//      }
//
//  template<typename _Tp, typename _Alloc>
//    void
//    list<_Tp, _Alloc>::
//    remove(const value_type& __value)
//    {
//      iterator __first = begin();
//      iterator __last = end();
//      iterator __extra = __last;
//      while (__first != __last)
//        {
//          iterator __next = __first;
//          ++__next;
//          if (*__first == __value)
//            {
//              // _GLIBCXX_RESOLVE_LIB_DEFECTS
//              // 526. Is it undefined if a function in the standard changes
//              // in parameters?
//              if (std::__addressof(*__first) != std::__addressof(__value))
//                _M_erase(__first);
//              else
//                __extra = __first;
//            }
//          __first = __next;
//        }
//      if (__extra != __last)
//        _M_erase(__extra);
//    }
//
//  template<typename _Tp, typename _Alloc>
//    void
//    list<_Tp, _Alloc>::
//    unique()
//    {
//      iterator __first = begin();
//      iterator __last = end();
//      if (__first == __last)
//        return;
//      iterator __next = __first;
//      while (++__next != __last)
//        {
//          if (*__first == *__next)
//            _M_erase(__next);
//          else
//            __first = __next;
//          __next = __first;
//        }
//    }
//
//  template<typename _Tp, typename _Alloc>
//    void
//    list<_Tp, _Alloc>::
//#if __cplusplus >= 201103L
//    merge(list&& __x)
//#else
//    merge(list& __x)
//#endif
//    {
//      // _GLIBCXX_RESOLVE_LIB_DEFECTS
//      // 300. list::merge() specification incomplete
//      if (this != &__x)
//        {
//          _M_check_equal_allocators(__x); 
//
//          iterator __first1 = begin();
//          iterator __last1 = end();
//          iterator __first2 = __x.begin();
//          iterator __last2 = __x.end();
//          while (__first1 != __last1 && __first2 != __last2)
//            if (*__first2 < *__first1)
//              {
//                iterator __next = __first2;
//                _M_transfer(__first1, __first2, ++__next);
//                __first2 = __next;
//              }
//            else
//              ++__first1;
//          if (__first2 != __last2)
//            _M_transfer(__last1, __first2, __last2);
//        }
//    }
//
//  template<typename _Tp, typename _Alloc>
//    template <typename _StrictWeakOrdering>
//      void
//      list<_Tp, _Alloc>::
//#if __cplusplus >= 201103L
//      merge(list&& __x, _StrictWeakOrdering __comp)
//#else
//      merge(list& __x, _StrictWeakOrdering __comp)
//#endif
//      {
//        // _GLIBCXX_RESOLVE_LIB_DEFECTS
//        // 300. list::merge() specification incomplete
//        if (this != &__x)
//          {
//            _M_check_equal_allocators(__x);
//
//            iterator __first1 = begin();
//            iterator __last1 = end();
//            iterator __first2 = __x.begin();
//            iterator __last2 = __x.end();
//            while (__first1 != __last1 && __first2 != __last2)
//              if (__comp(*__first2, *__first1))
//                {
//                  iterator __next = __first2;
//                  _M_transfer(__first1, __first2, ++__next);
//                  __first2 = __next;
//                }
//              else
//                ++__first1;
//            if (__first2 != __last2)
//              _M_transfer(__last1, __first2, __last2);
//          }
//      }
//
//  template<typename _Tp, typename _Alloc>
//    void
//    list<_Tp, _Alloc>::
//    sort()
//    {
//      // Do nothing if the list has length 0 or 1.
//      if (this->_M_impl._M_node._M_next != &this->_M_impl._M_node
//          && this->_M_impl._M_node._M_next->_M_next != &this->_M_impl._M_node)
//      {
//        list __carry;
//        list __tmp[64];
//        list * __fill = &__tmp[0];
//        list * __counter;
//
//        do
//          {
//            __carry.splice(__carry.begin(), *this, begin());
//
//            for(__counter = &__tmp[0];
//                __counter != __fill && !__counter->empty();
//                ++__counter)
//              {
//                __counter->merge(__carry);
//                __carry.swap(*__counter);
//              }
//            __carry.swap(*__counter);
//            if (__counter == __fill)
//              ++__fill;
//          }
//        while ( !empty() );
//
//        for (__counter = &__tmp[1]; __counter != __fill; ++__counter)
//          __counter->merge(*(__counter - 1));
//        swap( *(__fill - 1) );
//      }
//    }
//
//  template<typename _Tp, typename _Alloc>
//    template <typename _Predicate>
//      void
//      list<_Tp, _Alloc>::
//      remove_if(_Predicate __pred)
//      {
//        iterator __first = begin();
//        iterator __last = end();
//        while (__first != __last)
//          {
//            iterator __next = __first;
//            ++__next;
//            if (__pred(*__first))
//              _M_erase(__first);
//            __first = __next;
//          }
//      }
//
//  template<typename _Tp, typename _Alloc>
//    template <typename _BinaryPredicate>
//      void
//      list<_Tp, _Alloc>::
//      unique(_BinaryPredicate __binary_pred)
//      {
//        iterator __first = begin();
//        iterator __last = end();
//        if (__first == __last)
//          return;
//        iterator __next = __first;
//        while (++__next != __last)
//          {
//            if (__binary_pred(*__first, *__next))
//              _M_erase(__next);
//            else
//              __first = __next;
//            __next = __first;
//          }
//      }
//
//  template<typename _Tp, typename _Alloc>
//    template <typename _StrictWeakOrdering>
//      void
//      list<_Tp, _Alloc>::
//      sort(_StrictWeakOrdering __comp)
//      {
//        // Do nothing if the list has length 0 or 1.
//        if (this->_M_impl._M_node._M_next != &this->_M_impl._M_node
//            && this->_M_impl._M_node._M_next->_M_next != &this->_M_impl._M_node)
//          {
//            list __carry;
//            list __tmp[64];
//            list * __fill = &__tmp[0];
//            list * __counter;
//
//            do
//              {
//                __carry.splice(__carry.begin(), *this, begin());
//
//                for(__counter = &__tmp[0];
//                    __counter != __fill && !__counter->empty();
//                    ++__counter)
//                  {
//                    __counter->merge(__carry, __comp);
//                    __carry.swap(*__counter);
//                  }
//                __carry.swap(*__counter);
//                if (__counter == __fill)
//                  ++__fill;
//              }
//            while ( !empty() );
//
//            for (__counter = &__tmp[1]; __counter != __fill; ++__counter)
//              __counter->merge(*(__counter - 1), __comp);
//            swap(*(__fill - 1));
//          }
//      }
//
//_GLIBCXX_END_NAMESPACE_CONTAINER
//} // namespace std
//
//#endif /* _LIST_TCC */
//
//
//
//
///*
//#ifdef _GLIBCXX_DEBUG
//# include <debug/list>
//#endif
//
//#ifdef _GLIBCXX_PROFILE
//# include <profile/list>
//#endif
//
//#endif /* _GLIBCXX_LIST */
