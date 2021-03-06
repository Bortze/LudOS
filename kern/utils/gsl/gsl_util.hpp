///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2015 Microsoft Corporation. All rights reserved.
//
// This code is licensed under the MIT License (MIT).
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef GSL_UTIL_H
#define GSL_UTIL_H

#include <stdlib.h>

#include <utils/gsl/gsl_assert.hpp> // for Expects

#include <array.hpp>
#include <stddef.h>          // for ptrdiff_t, size_t
#include <initializer_list.hpp> // for initializer_list
#include <type_traits.hpp>      // for is_signed, integral_constant
#include <utility.hpp>          // for forward

#if defined(_MSC_VER)

#pragma warning(push)
#pragma warning(disable : 4127) // conditional expression is constant

#if _MSC_VER < 1910
#pragma push_macro("constexpr")
#define constexpr /*constexpr*/
#endif                          // _MSC_VER < 1910
#endif                          // _MSC_VER

namespace gsl
{
//
// GSL.util: utilities
//

// final_action allows you to ensure something gets run at the end of a scope
template <class F>
class final_action
{
public:
    explicit final_action(F f) noexcept : f_(std::move(f)) {}

    final_action(final_action&& other) noexcept : f_(std::move(other.f_)), invoke_(other.invoke_)
    {
        other.invoke_ = false;
    }

    final_action(const final_action&) = delete;
    final_action& operator=(const final_action&) = delete;

    ~final_action() noexcept
    {
        if (invoke_) f_();
    }

private:
    F f_;
    bool invoke_ {true};
};

// finally() - convenience function to generate a final_action
template <class F>
inline final_action<F> finally(const F& f) noexcept
{
    return final_action<F>(f);
}

template <class F>
inline final_action<F> finally(F&& f) noexcept
{
    return final_action<F>(std::forward<F>(f));
}

// narrow_cast(): a searchable way to do narrowing casts of values
template <class T, class U>
inline constexpr T narrow_cast(U&& u) noexcept
{
    return static_cast<T>(std::forward<U>(u));
}

namespace details
{
    template <class T, class U>
    struct is_same_signedness
        : public std::integral_constant<bool, std::is_signed<T>::value == std::is_signed<U>::value>
    {
    };
}

// narrow() : a checked version of narrow_cast() that throws if the cast changed the value
template <class T, class U>
inline T narrow(U u)
{
    T t = narrow_cast<T>(u);
#ifdef __EXCEPTIONS
    if (static_cast<U>(t) != u) throw narrowing_error();
    if (!details::is_same_signedness<T, U>::value && ((t < T{}) != (u < U{})))
        throw narrowing_error();
#else
    if (static_cast<U>(t) != u) abort();
    if (!details::is_same_signedness<T, U>::value && ((t < T{}) != (u < U{})))
        abort();
#endif
    return t;
}

//
// at() - Bounds-checked way of accessing builtin arrays, std::array, std::vector
//
template <class T, size_t N>
inline constexpr T& at(T (&arr)[N], const ptrdiff_t index)
{
    Expects(index >= 0 && index < narrow_cast<ptrdiff_t>(N));
    return arr[static_cast<size_t>(index)];
}

template <class Cont>
inline constexpr auto at(Cont& cont, const ptrdiff_t index) -> decltype(cont[cont.size()])
{
    Expects(index >= 0 && index < narrow_cast<ptrdiff_t>(cont.size()));
    using size_type = decltype(cont.size());
    return cont[static_cast<size_type>(index)];
}

template <class T>
inline constexpr T at(const std::initializer_list<T> cont, const ptrdiff_t index)
{
    Expects(index >= 0 && index < narrow_cast<ptrdiff_t>(cont.size()));
    return *(cont.begin() + index);
}

} // namespace gsl

#if defined(_MSC_VER)
#if _MSC_VER < 1910
#undef constexpr
#pragma pop_macro("constexpr")

#endif // _MSC_VER < 1910

#pragma warning(pop)

#endif // _MSC_VER

#endif // GSL_UTIL_H
