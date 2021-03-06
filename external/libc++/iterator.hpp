// -*- C++ -*-
//===-------------------------- iterator ----------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP_ITERATOR
#define _LIBCPP_ITERATOR


#include "__config.hpp"
#include "__functional_base.hpp"
#include "type_traits.hpp"
#include <stddef.h>
#include "initializer_list.hpp"
#ifdef __APPLE__
#include <Availability.h>
#endif

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#pragma GCC system_header
#endif

_LIBCPP_BEGIN_NAMESPACE_STD

struct _LIBCPP_TEMPLATE_VIS input_iterator_tag {};
struct _LIBCPP_TEMPLATE_VIS output_iterator_tag {};
struct _LIBCPP_TEMPLATE_VIS forward_iterator_tag       : public input_iterator_tag {};
struct _LIBCPP_TEMPLATE_VIS bidirectional_iterator_tag : public forward_iterator_tag {};
struct _LIBCPP_TEMPLATE_VIS random_access_iterator_tag : public bidirectional_iterator_tag {};

template <class _Tp>
struct __has_iterator_category
{
private:
    struct __two {char __lx; char __lxx;};
    template <class _Up> static __two __test(...);
    template <class _Up> static char __test(typename _Up::iterator_category* = 0);
public:
    static const bool value = sizeof(__test<_Tp>(0)) == 1;
};

template <class _Iter, bool> struct __iterator_traits_impl {};

template <class _Iter>
struct __iterator_traits_impl<_Iter, true>
{
    typedef typename _Iter::difference_type   difference_type;
    typedef typename _Iter::value_type        value_type;
    typedef typename _Iter::pointer           pointer;
    typedef typename _Iter::reference         reference;
    typedef typename _Iter::iterator_category iterator_category;
};

template <class _Iter, bool> struct __iterator_traits {};

template <class _Iter>
struct __iterator_traits<_Iter, true>
    :  __iterator_traits_impl
      <
        _Iter,
        is_convertible<typename _Iter::iterator_category, input_iterator_tag>::value ||
        is_convertible<typename _Iter::iterator_category, output_iterator_tag>::value
      >
{};

// iterator_traits<Iterator> will only have the nested types if Iterator::iterator_category
//    exists.  Else iterator_traits<Iterator> will be an empty class.  This is a
//    conforming extension which allows some programs to compile and behave as
//    the client expects instead of failing at compile time.

template <class _Iter>
struct _LIBCPP_TEMPLATE_VIS iterator_traits
    : __iterator_traits<_Iter, __has_iterator_category<_Iter>::value> {};

template<class _Tp>
struct _LIBCPP_TEMPLATE_VIS iterator_traits<_Tp*>
{
    typedef ptrdiff_t difference_type;
    typedef typename remove_const<_Tp>::type value_type;
    typedef _Tp* pointer;
    typedef _Tp& reference;
    typedef random_access_iterator_tag iterator_category;
};

template <class _Tp, class _Up, bool = __has_iterator_category<iterator_traits<_Tp> >::value>
struct __has_iterator_category_convertible_to
    : public integral_constant<bool, is_convertible<typename iterator_traits<_Tp>::iterator_category, _Up>::value>
{};

template <class _Tp, class _Up>
struct __has_iterator_category_convertible_to<_Tp, _Up, false> : public false_type {};

template <class _Tp>
struct __is_input_iterator : public __has_iterator_category_convertible_to<_Tp, input_iterator_tag> {};

template <class _Tp>
struct __is_forward_iterator : public __has_iterator_category_convertible_to<_Tp, forward_iterator_tag> {};

template <class _Tp>
struct __is_bidirectional_iterator : public __has_iterator_category_convertible_to<_Tp, bidirectional_iterator_tag> {};

template <class _Tp>
struct __is_random_access_iterator : public __has_iterator_category_convertible_to<_Tp, random_access_iterator_tag> {};

template <class _Tp>
struct __is_exactly_input_iterator
    : public integral_constant<bool,
         __has_iterator_category_convertible_to<_Tp, input_iterator_tag>::value &&
        !__has_iterator_category_convertible_to<_Tp, forward_iterator_tag>::value> {};

template<class _Category, class _Tp, class _Distance = ptrdiff_t,
         class _Pointer = _Tp*, class _Reference = _Tp&>
struct _LIBCPP_TEMPLATE_VIS iterator
{
    typedef _Tp        value_type;
    typedef _Distance  difference_type;
    typedef _Pointer   pointer;
    typedef _Reference reference;
    typedef _Category  iterator_category;
};

template <class _InputIter>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
void __advance(_InputIter& __i,
             typename iterator_traits<_InputIter>::difference_type __n, input_iterator_tag)
{
    for (; __n > 0; --__n)
        ++__i;
}

template <class _BiDirIter>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
void __advance(_BiDirIter& __i,
             typename iterator_traits<_BiDirIter>::difference_type __n, bidirectional_iterator_tag)
{
    if (__n >= 0)
        for (; __n > 0; --__n)
            ++__i;
    else
        for (; __n < 0; ++__n)
            --__i;
}

template <class _RandIter>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
void __advance(_RandIter& __i,
             typename iterator_traits<_RandIter>::difference_type __n, random_access_iterator_tag)
{
   __i += __n;
}

template <class _InputIter>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
void advance(_InputIter& __i,
             typename iterator_traits<_InputIter>::difference_type __n)
{
    __advance(__i, __n, typename iterator_traits<_InputIter>::iterator_category());
}

template <class _InputIter>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
typename iterator_traits<_InputIter>::difference_type
__distance(_InputIter __first, _InputIter __last, input_iterator_tag)
{
    typename iterator_traits<_InputIter>::difference_type __r(0);
    for (; __first != __last; ++__first)
        ++__r;
    return __r;
}

template <class _RandIter>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
typename iterator_traits<_RandIter>::difference_type
__distance(_RandIter __first, _RandIter __last, random_access_iterator_tag)
{
    return __last - __first;
}

template <class _InputIter>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
typename iterator_traits<_InputIter>::difference_type
distance(_InputIter __first, _InputIter __last)
{
    return __distance(__first, __last, typename iterator_traits<_InputIter>::iterator_category());
}

template <class _InputIter>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
typename enable_if
<
    __is_input_iterator<_InputIter>::value,
    _InputIter
>::type
next(_InputIter __x,
     typename iterator_traits<_InputIter>::difference_type __n = 1)
{
    _VSTD::advance(__x, __n);
    return __x;
}

template <class _BidirectionalIter>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
typename enable_if
<
    __is_bidirectional_iterator<_BidirectionalIter>::value,
    _BidirectionalIter
>::type
prev(_BidirectionalIter __x,
     typename iterator_traits<_BidirectionalIter>::difference_type __n = 1)
{
    _VSTD::advance(__x, -__n);
    return __x;
}


template <class _Tp, class = void>
struct __is_stashing_iterator : false_type {};

template <class _Tp>
struct __is_stashing_iterator<_Tp, typename __void_t<typename _Tp::__stashing_iterator_tag>::type>
  : true_type {};

template <class _Iter>
class _LIBCPP_TEMPLATE_VIS reverse_iterator
    : public iterator<typename iterator_traits<_Iter>::iterator_category,
                      typename iterator_traits<_Iter>::value_type,
                      typename iterator_traits<_Iter>::difference_type,
                      typename iterator_traits<_Iter>::pointer,
                      typename iterator_traits<_Iter>::reference>
{
private:
    /*mutable*/ _Iter __t;  // no longer used as of LWG #2360, not removed due to ABI break

    static_assert(!__is_stashing_iterator<_Iter>::value,
      "The specified iterator type cannot be used with reverse_iterator; "
      "Using stashing iterators with reverse_iterator causes undefined behavior");

protected:
    _Iter current;
public:
    typedef _Iter                                            iterator_type;
    typedef typename iterator_traits<_Iter>::difference_type difference_type;
    typedef typename iterator_traits<_Iter>::reference       reference;
    typedef typename iterator_traits<_Iter>::pointer         pointer;

    _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
    reverse_iterator() : __t(), current() {}
    _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
    explicit reverse_iterator(_Iter __x) : __t(__x), current(__x) {}
    template <class _Up>
        _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
        reverse_iterator(const reverse_iterator<_Up>& __u) : __t(__u.base()), current(__u.base()) {}
    template <class _Up>
        _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
        reverse_iterator& operator=(const reverse_iterator<_Up>& __u)
            { __t = current = __u.base(); return *this; }
    _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
    _Iter base() const {return current;}
    _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
    reference operator*() const {_Iter __tmp = current; return *--__tmp;}
    _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
    pointer  operator->() const {return _VSTD::addressof(operator*());}
    _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
    reverse_iterator& operator++() {--current; return *this;}
    _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
    reverse_iterator  operator++(int) {reverse_iterator __tmp(*this); --current; return __tmp;}
    _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
    reverse_iterator& operator--() {++current; return *this;}
    _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
    reverse_iterator  operator--(int) {reverse_iterator __tmp(*this); ++current; return __tmp;}
    _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
    reverse_iterator  operator+ (difference_type __n) const {return reverse_iterator(current - __n);}
    _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
    reverse_iterator& operator+=(difference_type __n) {current -= __n; return *this;}
    _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
    reverse_iterator  operator- (difference_type __n) const {return reverse_iterator(current + __n);}
    _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
    reverse_iterator& operator-=(difference_type __n) {current += __n; return *this;}
    _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
    reference         operator[](difference_type __n) const {return *(*this + __n);}
};

template <class _Iter1, class _Iter2>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
bool
operator==(const reverse_iterator<_Iter1>& __x, const reverse_iterator<_Iter2>& __y)
{
    return __x.base() == __y.base();
}

template <class _Iter1, class _Iter2>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
bool
operator<(const reverse_iterator<_Iter1>& __x, const reverse_iterator<_Iter2>& __y)
{
    return __x.base() > __y.base();
}

template <class _Iter1, class _Iter2>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
bool
operator!=(const reverse_iterator<_Iter1>& __x, const reverse_iterator<_Iter2>& __y)
{
    return __x.base() != __y.base();
}

template <class _Iter1, class _Iter2>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
bool
operator>(const reverse_iterator<_Iter1>& __x, const reverse_iterator<_Iter2>& __y)
{
    return __x.base() < __y.base();
}

template <class _Iter1, class _Iter2>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
bool
operator>=(const reverse_iterator<_Iter1>& __x, const reverse_iterator<_Iter2>& __y)
{
    return __x.base() <= __y.base();
}

template <class _Iter1, class _Iter2>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
bool
operator<=(const reverse_iterator<_Iter1>& __x, const reverse_iterator<_Iter2>& __y)
{
    return __x.base() >= __y.base();
}

#ifndef _LIBCPP_CXX03_LANG
template <class _Iter1, class _Iter2>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
auto
operator-(const reverse_iterator<_Iter1>& __x, const reverse_iterator<_Iter2>& __y)
-> decltype(__y.base() - __x.base())
{
    return __y.base() - __x.base();
}
#else
template <class _Iter1, class _Iter2>
inline _LIBCPP_INLINE_VISIBILITY
typename reverse_iterator<_Iter1>::difference_type
operator-(const reverse_iterator<_Iter1>& __x, const reverse_iterator<_Iter2>& __y)
{
    return __y.base() - __x.base();
}
#endif

template <class _Iter>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
reverse_iterator<_Iter>
operator+(typename reverse_iterator<_Iter>::difference_type __n, const reverse_iterator<_Iter>& __x)
{
    return reverse_iterator<_Iter>(__x.base() - __n);
}

#if _LIBCPP_STD_VER > 11
template <class _Iter>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
reverse_iterator<_Iter> make_reverse_iterator(_Iter __i)
{
    return reverse_iterator<_Iter>(__i);
}
#endif

template <class _Container>
class _LIBCPP_TEMPLATE_VIS back_insert_iterator
    : public iterator<output_iterator_tag,
                      void,
                      void,
                      void,
                      void>
{
protected:
    _Container* container;
public:
    typedef _Container container_type;

    _LIBCPP_INLINE_VISIBILITY explicit back_insert_iterator(_Container& __x) : container(_VSTD::addressof(__x)) {}
    _LIBCPP_INLINE_VISIBILITY back_insert_iterator& operator=(const typename _Container::value_type& __value_)
        {container->push_back(__value_); return *this;}
#ifndef _LIBCPP_CXX03_LANG
    _LIBCPP_INLINE_VISIBILITY back_insert_iterator& operator=(typename _Container::value_type&& __value_)
        {container->push_back(_VSTD::move(__value_)); return *this;}
#endif  // _LIBCPP_CXX03_LANG
    _LIBCPP_INLINE_VISIBILITY back_insert_iterator& operator*()     {return *this;}
    _LIBCPP_INLINE_VISIBILITY back_insert_iterator& operator++()    {return *this;}
    _LIBCPP_INLINE_VISIBILITY back_insert_iterator  operator++(int) {return *this;}
};

template <class _Container>
inline _LIBCPP_INLINE_VISIBILITY
back_insert_iterator<_Container>
back_inserter(_Container& __x)
{
    return back_insert_iterator<_Container>(__x);
}

template <class _Container>
class _LIBCPP_TEMPLATE_VIS front_insert_iterator
    : public iterator<output_iterator_tag,
                      void,
                      void,
                      void,
                      void>
{
protected:
    _Container* container;
public:
    typedef _Container container_type;

    _LIBCPP_INLINE_VISIBILITY explicit front_insert_iterator(_Container& __x) : container(_VSTD::addressof(__x)) {}
    _LIBCPP_INLINE_VISIBILITY front_insert_iterator& operator=(const typename _Container::value_type& __value_)
        {container->push_front(__value_); return *this;}
#ifndef _LIBCPP_CXX03_LANG
    _LIBCPP_INLINE_VISIBILITY front_insert_iterator& operator=(typename _Container::value_type&& __value_)
        {container->push_front(_VSTD::move(__value_)); return *this;}
#endif  // _LIBCPP_CXX03_LANG
    _LIBCPP_INLINE_VISIBILITY front_insert_iterator& operator*()     {return *this;}
    _LIBCPP_INLINE_VISIBILITY front_insert_iterator& operator++()    {return *this;}
    _LIBCPP_INLINE_VISIBILITY front_insert_iterator  operator++(int) {return *this;}
};

template <class _Container>
inline _LIBCPP_INLINE_VISIBILITY
front_insert_iterator<_Container>
front_inserter(_Container& __x)
{
    return front_insert_iterator<_Container>(__x);
}

template <class _Container>
class _LIBCPP_TEMPLATE_VIS insert_iterator
    : public iterator<output_iterator_tag,
                      void,
                      void,
                      void,
                      void>
{
protected:
    _Container* container;
    typename _Container::iterator iter;
public:
    typedef _Container container_type;

    _LIBCPP_INLINE_VISIBILITY insert_iterator(_Container& __x, typename _Container::iterator __i)
        : container(_VSTD::addressof(__x)), iter(__i) {}
    _LIBCPP_INLINE_VISIBILITY insert_iterator& operator=(const typename _Container::value_type& __value_)
        {iter = container->insert(iter, __value_); ++iter; return *this;}
#ifndef _LIBCPP_CXX03_LANG
    _LIBCPP_INLINE_VISIBILITY insert_iterator& operator=(typename _Container::value_type&& __value_)
        {iter = container->insert(iter, _VSTD::move(__value_)); ++iter; return *this;}
#endif  // _LIBCPP_CXX03_LANG
    _LIBCPP_INLINE_VISIBILITY insert_iterator& operator*()        {return *this;}
    _LIBCPP_INLINE_VISIBILITY insert_iterator& operator++()       {return *this;}
    _LIBCPP_INLINE_VISIBILITY insert_iterator& operator++(int)    {return *this;}
};

template <class _Container>
inline _LIBCPP_INLINE_VISIBILITY
insert_iterator<_Container>
inserter(_Container& __x, typename _Container::iterator __i)
{
    return insert_iterator<_Container>(__x, __i);
}

template <class _Iter>
class _LIBCPP_TEMPLATE_VIS move_iterator
{
private:
    _Iter __i;
public:
    typedef _Iter                                            iterator_type;
    typedef typename iterator_traits<iterator_type>::iterator_category iterator_category;
    typedef typename iterator_traits<iterator_type>::value_type value_type;
    typedef typename iterator_traits<iterator_type>::difference_type difference_type;
    typedef iterator_type pointer;
#ifndef _LIBCPP_CXX03_LANG
    typedef typename iterator_traits<iterator_type>::reference __reference;
    typedef typename conditional<
            is_reference<__reference>::value,
            typename remove_reference<__reference>::type&&,
            __reference
        >::type reference;
#else
    typedef typename iterator_traits<iterator_type>::reference reference;
#endif

    _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
    move_iterator() : __i() {}
    _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
    explicit move_iterator(_Iter __x) : __i(__x) {}
    template <class _Up>
      _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
      move_iterator(const move_iterator<_Up>& __u) : __i(__u.base()) {}
    _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14 _Iter base() const {return __i;}
    _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
    reference operator*() const { return static_cast<reference>(*__i); }
    _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
    pointer  operator->() const { return __i;}
    _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
    move_iterator& operator++() {++__i; return *this;}
    _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
    move_iterator  operator++(int) {move_iterator __tmp(*this); ++__i; return __tmp;}
    _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
    move_iterator& operator--() {--__i; return *this;}
    _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
    move_iterator  operator--(int) {move_iterator __tmp(*this); --__i; return __tmp;}
    _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
    move_iterator  operator+ (difference_type __n) const {return move_iterator(__i + __n);}
    _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
    move_iterator& operator+=(difference_type __n) {__i += __n; return *this;}
    _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
    move_iterator  operator- (difference_type __n) const {return move_iterator(__i - __n);}
    _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
    move_iterator& operator-=(difference_type __n) {__i -= __n; return *this;}
    _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
    reference operator[](difference_type __n) const { return static_cast<reference>(__i[__n]); }
};

template <class _Iter1, class _Iter2>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
bool
operator==(const move_iterator<_Iter1>& __x, const move_iterator<_Iter2>& __y)
{
    return __x.base() == __y.base();
}

template <class _Iter1, class _Iter2>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
bool
operator<(const move_iterator<_Iter1>& __x, const move_iterator<_Iter2>& __y)
{
    return __x.base() < __y.base();
}

template <class _Iter1, class _Iter2>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
bool
operator!=(const move_iterator<_Iter1>& __x, const move_iterator<_Iter2>& __y)
{
    return __x.base() != __y.base();
}

template <class _Iter1, class _Iter2>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
bool
operator>(const move_iterator<_Iter1>& __x, const move_iterator<_Iter2>& __y)
{
    return __x.base() > __y.base();
}

template <class _Iter1, class _Iter2>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
bool
operator>=(const move_iterator<_Iter1>& __x, const move_iterator<_Iter2>& __y)
{
    return __x.base() >= __y.base();
}

template <class _Iter1, class _Iter2>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
bool
operator<=(const move_iterator<_Iter1>& __x, const move_iterator<_Iter2>& __y)
{
    return __x.base() <= __y.base();
}

#ifndef _LIBCPP_CXX03_LANG
template <class _Iter1, class _Iter2>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
auto
operator-(const move_iterator<_Iter1>& __x, const move_iterator<_Iter2>& __y)
-> decltype(__x.base() - __y.base())
{
    return __x.base() - __y.base();
}
#else
template <class _Iter1, class _Iter2>
inline _LIBCPP_INLINE_VISIBILITY
typename move_iterator<_Iter1>::difference_type
operator-(const move_iterator<_Iter1>& __x, const move_iterator<_Iter2>& __y)
{
    return __x.base() - __y.base();
}
#endif

template <class _Iter>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
move_iterator<_Iter>
operator+(typename move_iterator<_Iter>::difference_type __n, const move_iterator<_Iter>& __x)
{
    return move_iterator<_Iter>(__x.base() + __n);
}

template <class _Iter>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
move_iterator<_Iter>
make_move_iterator(_Iter __i)
{
    return move_iterator<_Iter>(__i);
}

// __wrap_iter

template <class _Iter> class __wrap_iter;

template <class _Iter1, class _Iter2>
_LIBCPP_INLINE_VISIBILITY
bool
operator==(const __wrap_iter<_Iter1>&, const __wrap_iter<_Iter2>&) _NOEXCEPT_DEBUG;

template <class _Iter1, class _Iter2>
_LIBCPP_INLINE_VISIBILITY
bool
operator<(const __wrap_iter<_Iter1>&, const __wrap_iter<_Iter2>&) _NOEXCEPT_DEBUG;

template <class _Iter1, class _Iter2>
_LIBCPP_INLINE_VISIBILITY
bool
operator!=(const __wrap_iter<_Iter1>&, const __wrap_iter<_Iter2>&) _NOEXCEPT_DEBUG;

template <class _Iter1, class _Iter2>
_LIBCPP_INLINE_VISIBILITY
bool
operator>(const __wrap_iter<_Iter1>&, const __wrap_iter<_Iter2>&) _NOEXCEPT_DEBUG;

template <class _Iter1, class _Iter2>
_LIBCPP_INLINE_VISIBILITY
bool
operator>=(const __wrap_iter<_Iter1>&, const __wrap_iter<_Iter2>&) _NOEXCEPT_DEBUG;

template <class _Iter1, class _Iter2>
_LIBCPP_INLINE_VISIBILITY
bool
operator<=(const __wrap_iter<_Iter1>&, const __wrap_iter<_Iter2>&) _NOEXCEPT_DEBUG;

#ifndef _LIBCPP_CXX03_LANG
template <class _Iter1, class _Iter2>
_LIBCPP_INLINE_VISIBILITY
auto
operator-(const __wrap_iter<_Iter1>& __x, const __wrap_iter<_Iter2>& __y) _NOEXCEPT_DEBUG
-> decltype(__x.base() - __y.base());
#else
template <class _Iter1, class _Iter2>
_LIBCPP_INLINE_VISIBILITY
typename __wrap_iter<_Iter1>::difference_type
operator-(const __wrap_iter<_Iter1>&, const __wrap_iter<_Iter2>&) _NOEXCEPT_DEBUG;
#endif

template <class _Iter>
_LIBCPP_INLINE_VISIBILITY
__wrap_iter<_Iter>
operator+(typename __wrap_iter<_Iter>::difference_type, __wrap_iter<_Iter>) _NOEXCEPT_DEBUG;

template <class _Ip, class _Op> _Op _LIBCPP_INLINE_VISIBILITY copy(_Ip, _Ip, _Op);
template <class _B1, class _B2> _B2 _LIBCPP_INLINE_VISIBILITY copy_backward(_B1, _B1, _B2);
template <class _Ip, class _Op> _Op _LIBCPP_INLINE_VISIBILITY move(_Ip, _Ip, _Op);
template <class _B1, class _B2> _B2 _LIBCPP_INLINE_VISIBILITY move_backward(_B1, _B1, _B2);

#if _LIBCPP_DEBUG_LEVEL < 2

template <class _Tp>
_LIBCPP_INLINE_VISIBILITY
typename enable_if
<
    is_trivially_copy_assignable<_Tp>::value,
    _Tp*
>::type
__unwrap_iter(__wrap_iter<_Tp*>);

#else

template <class _Tp>
inline _LIBCPP_INLINE_VISIBILITY
typename enable_if
<
    is_trivially_copy_assignable<_Tp>::value,
    __wrap_iter<_Tp*>
>::type
__unwrap_iter(__wrap_iter<_Tp*> __i);

#endif

template <class _Iter>
class __wrap_iter
{
public:
    typedef _Iter                                                      iterator_type;
    typedef typename iterator_traits<iterator_type>::iterator_category iterator_category;
    typedef typename iterator_traits<iterator_type>::value_type        value_type;
    typedef typename iterator_traits<iterator_type>::difference_type   difference_type;
    typedef typename iterator_traits<iterator_type>::pointer           pointer;
    typedef typename iterator_traits<iterator_type>::reference         reference;
private:
    iterator_type __i;
public:
    _LIBCPP_INLINE_VISIBILITY __wrap_iter() _NOEXCEPT_DEBUG
#if _LIBCPP_STD_VER > 11
                : __i{}
#endif
    {
#if _LIBCPP_DEBUG_LEVEL >= 2
        __get_db()->__insert_i(this);
#endif
    }
    template <class _Up> _LIBCPP_INLINE_VISIBILITY __wrap_iter(const __wrap_iter<_Up>& __u,
        typename enable_if<is_convertible<_Up, iterator_type>::value>::type* = 0) _NOEXCEPT_DEBUG
        : __i(__u.base())
    {
#if _LIBCPP_DEBUG_LEVEL >= 2
        __get_db()->__iterator_copy(this, &__u);
#endif
    }
#if _LIBCPP_DEBUG_LEVEL >= 2
    _LIBCPP_INLINE_VISIBILITY
    __wrap_iter(const __wrap_iter& __x)
        : __i(__x.base())
    {
        __get_db()->__iterator_copy(this, &__x);
    }
    _LIBCPP_INLINE_VISIBILITY
    __wrap_iter& operator=(const __wrap_iter& __x)
    {
        if (this != &__x)
        {
            __get_db()->__iterator_copy(this, &__x);
            __i = __x.__i;
        }
        return *this;
    }
    _LIBCPP_INLINE_VISIBILITY
    ~__wrap_iter()
    {
        __get_db()->__erase_i(this);
    }
#endif
    _LIBCPP_INLINE_VISIBILITY reference operator*() const _NOEXCEPT_DEBUG
    {
#if _LIBCPP_DEBUG_LEVEL >= 2
        _LIBCPP_ASSERT(__get_const_db()->__dereferenceable(this),
                       "Attempted to dereference a non-dereferenceable iterator");
#endif
        return *__i;
    }
    _LIBCPP_INLINE_VISIBILITY pointer  operator->() const _NOEXCEPT_DEBUG
    {
#if _LIBCPP_DEBUG_LEVEL >= 2
        _LIBCPP_ASSERT(__get_const_db()->__dereferenceable(this),
                       "Attempted to dereference a non-dereferenceable iterator");
#endif
        return (pointer)_VSTD::addressof(*__i);
    }
    _LIBCPP_INLINE_VISIBILITY __wrap_iter& operator++() _NOEXCEPT_DEBUG
    {
#if _LIBCPP_DEBUG_LEVEL >= 2
        _LIBCPP_ASSERT(__get_const_db()->__dereferenceable(this),
                       "Attempted to increment non-incrementable iterator");
#endif
        ++__i;
        return *this;
    }
    _LIBCPP_INLINE_VISIBILITY __wrap_iter  operator++(int) _NOEXCEPT_DEBUG
        {__wrap_iter __tmp(*this); ++(*this); return __tmp;}
    _LIBCPP_INLINE_VISIBILITY __wrap_iter& operator--() _NOEXCEPT_DEBUG
    {
#if _LIBCPP_DEBUG_LEVEL >= 2
        _LIBCPP_ASSERT(__get_const_db()->__decrementable(this),
                       "Attempted to decrement non-decrementable iterator");
#endif
        --__i;
        return *this;
    }
    _LIBCPP_INLINE_VISIBILITY __wrap_iter  operator--(int) _NOEXCEPT_DEBUG
        {__wrap_iter __tmp(*this); --(*this); return __tmp;}
    _LIBCPP_INLINE_VISIBILITY __wrap_iter  operator+ (difference_type __n) const _NOEXCEPT_DEBUG
        {__wrap_iter __w(*this); __w += __n; return __w;}
    _LIBCPP_INLINE_VISIBILITY __wrap_iter& operator+=(difference_type __n) _NOEXCEPT_DEBUG
    {
#if _LIBCPP_DEBUG_LEVEL >= 2
        _LIBCPP_ASSERT(__get_const_db()->__addable(this, __n),
                   "Attempted to add/subtract iterator outside of valid range");
#endif
        __i += __n;
        return *this;
    }
    _LIBCPP_INLINE_VISIBILITY __wrap_iter  operator- (difference_type __n) const _NOEXCEPT_DEBUG
        {return *this + (-__n);}
    _LIBCPP_INLINE_VISIBILITY __wrap_iter& operator-=(difference_type __n) _NOEXCEPT_DEBUG
        {*this += -__n; return *this;}
    _LIBCPP_INLINE_VISIBILITY reference        operator[](difference_type __n) const _NOEXCEPT_DEBUG
    {
#if _LIBCPP_DEBUG_LEVEL >= 2
        _LIBCPP_ASSERT(__get_const_db()->__subscriptable(this, __n),
                   "Attempted to subscript iterator outside of valid range");
#endif
        return __i[__n];
    }

    _LIBCPP_INLINE_VISIBILITY iterator_type base() const _NOEXCEPT_DEBUG {return __i;}

private:
#if _LIBCPP_DEBUG_LEVEL >= 2
    _LIBCPP_INLINE_VISIBILITY __wrap_iter(const void* __p, iterator_type __x) : __i(__x)
    {
        __get_db()->__insert_ic(this, __p);
    }
#else
    _LIBCPP_INLINE_VISIBILITY __wrap_iter(iterator_type __x) _NOEXCEPT_DEBUG : __i(__x) {}
#endif

    template <class _Up> friend class __wrap_iter;
    template <class _CharT, class _Traits, class _Alloc> friend class basic_string;
    template <class _Tp, class _Alloc> friend class _LIBCPP_TEMPLATE_VIS vector;

    template <class _Iter1, class _Iter2>
    friend
    bool
    operator==(const __wrap_iter<_Iter1>&, const __wrap_iter<_Iter2>&) _NOEXCEPT_DEBUG;

    template <class _Iter1, class _Iter2>
    friend
    bool
    operator<(const __wrap_iter<_Iter1>&, const __wrap_iter<_Iter2>&) _NOEXCEPT_DEBUG;

    template <class _Iter1, class _Iter2>
    friend
    bool
    operator!=(const __wrap_iter<_Iter1>&, const __wrap_iter<_Iter2>&) _NOEXCEPT_DEBUG;

    template <class _Iter1, class _Iter2>
    friend
    bool
    operator>(const __wrap_iter<_Iter1>&, const __wrap_iter<_Iter2>&) _NOEXCEPT_DEBUG;

    template <class _Iter1, class _Iter2>
    friend
    bool
    operator>=(const __wrap_iter<_Iter1>&, const __wrap_iter<_Iter2>&) _NOEXCEPT_DEBUG;

    template <class _Iter1, class _Iter2>
    friend
    bool
    operator<=(const __wrap_iter<_Iter1>&, const __wrap_iter<_Iter2>&) _NOEXCEPT_DEBUG;

#ifndef _LIBCPP_CXX03_LANG
    template <class _Iter1, class _Iter2>
    friend
    auto
    operator-(const __wrap_iter<_Iter1>& __x, const __wrap_iter<_Iter2>& __y) _NOEXCEPT_DEBUG
    -> decltype(__x.base() - __y.base());
#else
    template <class _Iter1, class _Iter2>
    friend
    typename __wrap_iter<_Iter1>::difference_type
    operator-(const __wrap_iter<_Iter1>&, const __wrap_iter<_Iter2>&) _NOEXCEPT_DEBUG;
#endif

    template <class _Iter1>
    friend
    __wrap_iter<_Iter1>
    operator+(typename __wrap_iter<_Iter1>::difference_type, __wrap_iter<_Iter1>) _NOEXCEPT_DEBUG;

    template <class _Ip, class _Op> friend _Op copy(_Ip, _Ip, _Op);
    template <class _B1, class _B2> friend _B2 copy_backward(_B1, _B1, _B2);
    template <class _Ip, class _Op> friend _Op move(_Ip, _Ip, _Op);
    template <class _B1, class _B2> friend _B2 move_backward(_B1, _B1, _B2);

#if _LIBCPP_DEBUG_LEVEL < 2
    template <class _Tp>
    friend
    typename enable_if
    <
        is_trivially_copy_assignable<_Tp>::value,
        _Tp*
    >::type
    __unwrap_iter(__wrap_iter<_Tp*>);
#else
  template <class _Tp>
  inline _LIBCPP_INLINE_VISIBILITY
  typename enable_if
  <
      is_trivially_copy_assignable<_Tp>::value,
      __wrap_iter<_Tp*>
  >::type
  __unwrap_iter(__wrap_iter<_Tp*> __i);
#endif
};

template <class _Iter1, class _Iter2>
inline _LIBCPP_INLINE_VISIBILITY
bool
operator==(const __wrap_iter<_Iter1>& __x, const __wrap_iter<_Iter2>& __y) _NOEXCEPT_DEBUG
{
    return __x.base() == __y.base();
}

template <class _Iter1, class _Iter2>
inline _LIBCPP_INLINE_VISIBILITY
bool
operator<(const __wrap_iter<_Iter1>& __x, const __wrap_iter<_Iter2>& __y) _NOEXCEPT_DEBUG
{
#if _LIBCPP_DEBUG_LEVEL >= 2
    _LIBCPP_ASSERT(__get_const_db()->__less_than_comparable(&__x, &__y),
                   "Attempted to compare incomparable iterators");
#endif
    return __x.base() < __y.base();
}

template <class _Iter1, class _Iter2>
inline _LIBCPP_INLINE_VISIBILITY
bool
operator!=(const __wrap_iter<_Iter1>& __x, const __wrap_iter<_Iter2>& __y) _NOEXCEPT_DEBUG
{
    return !(__x == __y);
}

template <class _Iter1, class _Iter2>
inline _LIBCPP_INLINE_VISIBILITY
bool
operator>(const __wrap_iter<_Iter1>& __x, const __wrap_iter<_Iter2>& __y) _NOEXCEPT_DEBUG
{
    return __y < __x;
}

template <class _Iter1, class _Iter2>
inline _LIBCPP_INLINE_VISIBILITY
bool
operator>=(const __wrap_iter<_Iter1>& __x, const __wrap_iter<_Iter2>& __y) _NOEXCEPT_DEBUG
{
    return !(__x < __y);
}

template <class _Iter1, class _Iter2>
inline _LIBCPP_INLINE_VISIBILITY
bool
operator<=(const __wrap_iter<_Iter1>& __x, const __wrap_iter<_Iter2>& __y) _NOEXCEPT_DEBUG
{
    return !(__y < __x);
}

template <class _Iter1>
inline _LIBCPP_INLINE_VISIBILITY
bool
operator!=(const __wrap_iter<_Iter1>& __x, const __wrap_iter<_Iter1>& __y) _NOEXCEPT_DEBUG
{
    return !(__x == __y);
}

template <class _Iter1>
inline _LIBCPP_INLINE_VISIBILITY
bool
operator>(const __wrap_iter<_Iter1>& __x, const __wrap_iter<_Iter1>& __y) _NOEXCEPT_DEBUG
{
    return __y < __x;
}

template <class _Iter1>
inline _LIBCPP_INLINE_VISIBILITY
bool
operator>=(const __wrap_iter<_Iter1>& __x, const __wrap_iter<_Iter1>& __y) _NOEXCEPT_DEBUG
{
    return !(__x < __y);
}

template <class _Iter1>
inline _LIBCPP_INLINE_VISIBILITY
bool
operator<=(const __wrap_iter<_Iter1>& __x, const __wrap_iter<_Iter1>& __y) _NOEXCEPT_DEBUG
{
    return !(__y < __x);
}

#ifndef _LIBCPP_CXX03_LANG
template <class _Iter1, class _Iter2>
inline _LIBCPP_INLINE_VISIBILITY
auto
operator-(const __wrap_iter<_Iter1>& __x, const __wrap_iter<_Iter2>& __y) _NOEXCEPT_DEBUG
-> decltype(__x.base() - __y.base())
{
#if _LIBCPP_DEBUG_LEVEL >= 2
    _LIBCPP_ASSERT(__get_const_db()->__less_than_comparable(&__x, &__y),
                   "Attempted to subtract incompatible iterators");
#endif
    return __x.base() - __y.base();
}
#else
template <class _Iter1, class _Iter2>
inline _LIBCPP_INLINE_VISIBILITY
typename __wrap_iter<_Iter1>::difference_type
operator-(const __wrap_iter<_Iter1>& __x, const __wrap_iter<_Iter2>& __y) _NOEXCEPT_DEBUG
{
#if _LIBCPP_DEBUG_LEVEL >= 2
    _LIBCPP_ASSERT(__get_const_db()->__less_than_comparable(&__x, &__y),
                   "Attempted to subtract incompatible iterators");
#endif
    return __x.base() - __y.base();
}
#endif

template <class _Iter>
inline _LIBCPP_INLINE_VISIBILITY
__wrap_iter<_Iter>
operator+(typename __wrap_iter<_Iter>::difference_type __n,
          __wrap_iter<_Iter> __x) _NOEXCEPT_DEBUG
{
    __x += __n;
    return __x;
}

template <class _Iter>
struct __libcpp_is_trivial_iterator
    : public _LIBCPP_BOOL_CONSTANT(is_pointer<_Iter>::value) {};

template <class _Iter>
struct __libcpp_is_trivial_iterator<move_iterator<_Iter> >
    : public _LIBCPP_BOOL_CONSTANT(__libcpp_is_trivial_iterator<_Iter>::value) {};

template <class _Iter>
struct __libcpp_is_trivial_iterator<reverse_iterator<_Iter> >
    : public _LIBCPP_BOOL_CONSTANT(__libcpp_is_trivial_iterator<_Iter>::value) {};

template <class _Iter>
struct __libcpp_is_trivial_iterator<__wrap_iter<_Iter> >
    : public _LIBCPP_BOOL_CONSTANT(__libcpp_is_trivial_iterator<_Iter>::value) {};


template <class _Tp, size_t _Np>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX11
_Tp*
begin(_Tp (&__array)[_Np])
{
    return __array;
}

template <class _Tp, size_t _Np>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX11
_Tp*
end(_Tp (&__array)[_Np])
{
    return __array + _Np;
}

#if !defined(_LIBCPP_CXX03_LANG)

template <class _Cp>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
auto
begin(_Cp& __c) -> decltype(__c.begin())
{
    return __c.begin();
}

template <class _Cp>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
auto
begin(const _Cp& __c) -> decltype(__c.begin())
{
    return __c.begin();
}

template <class _Cp>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
auto
end(_Cp& __c) -> decltype(__c.end())
{
    return __c.end();
}

template <class _Cp>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
auto
end(const _Cp& __c) -> decltype(__c.end())
{
    return __c.end();
}

#if _LIBCPP_STD_VER > 11

template <class _Tp, size_t _Np>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
reverse_iterator<_Tp*> rbegin(_Tp (&__array)[_Np])
{
    return reverse_iterator<_Tp*>(__array + _Np);
}

template <class _Tp, size_t _Np>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
reverse_iterator<_Tp*> rend(_Tp (&__array)[_Np])
{
    return reverse_iterator<_Tp*>(__array);
}

template <class _Ep>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
reverse_iterator<const _Ep*> rbegin(initializer_list<_Ep> __il)
{
    return reverse_iterator<const _Ep*>(__il.end());
}

template <class _Ep>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
reverse_iterator<const _Ep*> rend(initializer_list<_Ep> __il)
{
    return reverse_iterator<const _Ep*>(__il.begin());
}

template <class _Cp>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX11
auto cbegin(const _Cp& __c) -> decltype(_VSTD::begin(__c))
{
    return _VSTD::begin(__c);
}

template <class _Cp>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX11
auto cend(const _Cp& __c) -> decltype(_VSTD::end(__c))
{
    return _VSTD::end(__c);
}

template <class _Cp>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
auto rbegin(_Cp& __c) -> decltype(__c.rbegin())
{
    return __c.rbegin();
}

template <class _Cp>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
auto rbegin(const _Cp& __c) -> decltype(__c.rbegin())
{
    return __c.rbegin();
}

template <class _Cp>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
auto rend(_Cp& __c) -> decltype(__c.rend())
{
    return __c.rend();
}

template <class _Cp>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
auto rend(const _Cp& __c) -> decltype(__c.rend())
{
    return __c.rend();
}

template <class _Cp>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
auto crbegin(const _Cp& __c) -> decltype(_VSTD::rbegin(__c))
{
    return _VSTD::rbegin(__c);
}

template <class _Cp>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX14
auto crend(const _Cp& __c) -> decltype(_VSTD::rend(__c))
{
    return _VSTD::rend(__c);
}

#endif


#else  // defined(_LIBCPP_CXX03_LANG)

template <class _Cp>
inline _LIBCPP_INLINE_VISIBILITY
typename _Cp::iterator
begin(_Cp& __c)
{
    return __c.begin();
}

template <class _Cp>
inline _LIBCPP_INLINE_VISIBILITY
typename _Cp::const_iterator
begin(const _Cp& __c)
{
    return __c.begin();
}

template <class _Cp>
inline _LIBCPP_INLINE_VISIBILITY
typename _Cp::iterator
end(_Cp& __c)
{
    return __c.end();
}

template <class _Cp>
inline _LIBCPP_INLINE_VISIBILITY
typename _Cp::const_iterator
end(const _Cp& __c)
{
    return __c.end();
}

#endif  // !defined(_LIBCPP_CXX03_LANG)

#if _LIBCPP_STD_VER > 14
template <class _Cont>
constexpr auto size(const _Cont& __c) -> decltype(__c.size()) { return __c.size(); }

template <class _Tp, size_t _Sz>
constexpr size_t size(const _Tp (&)[_Sz]) noexcept { return _Sz; }

template <class _Cont>
constexpr auto empty(const _Cont& __c) -> decltype(__c.empty()) { return __c.empty(); }

template <class _Tp, size_t _Sz>
constexpr bool empty(const _Tp (&)[_Sz]) noexcept { return false; }

template <class _Ep>
constexpr bool empty(initializer_list<_Ep> __il) noexcept { return __il.size() == 0; }

template <class _Cont> constexpr
auto data(_Cont& __c) -> decltype(__c.data()) { return __c.data(); }

template <class _Cont> constexpr
auto data(const _Cont& __c) -> decltype(__c.data()) { return __c.data(); }

template <class _Tp, size_t _Sz>
constexpr _Tp* data(_Tp (&__array)[_Sz]) noexcept { return __array; }

template <class _Ep>
constexpr const _Ep* data(initializer_list<_Ep> __il) noexcept { return __il.begin(); }
#endif


_LIBCPP_END_NAMESPACE_STD

#endif  // _LIBCPP_ITERATOR
