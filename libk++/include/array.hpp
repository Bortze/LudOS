/*
array.hpp

Copyright (c) 30 Yann BOUCHER (yann)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
#ifndef ARRAY_HPP
#define ARRAY_HPP

#include <stdint.h>
#include <assert.h>

namespace kpp
{

template <typename T, size_t Len>
struct array
{
    T m_arr[Len];

    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = T*;
    using const_iterator = const T*;
    using difference_type = ptrdiff_t;
    using size_type = size_t;

    constexpr size_t size() const { return Len; }
    constexpr bool empty() const { return size() == 0; }

    constexpr T* begin() { return data(); }
    constexpr const T* begin() const { return data(); }

    constexpr T* end() { return data() + size(); }
    constexpr const T* end() const { return data() + size(); }

    constexpr T* data() { return m_arr; }
    constexpr const T* data() const { return m_arr; }

    constexpr T& operator[](size_t idx) { return data()[idx]; }
    constexpr const T& operator[](size_t idx) const { return data()[idx]; }

    constexpr T& at(size_t idx) { assert(idx < size());  return data()[idx]; }
    constexpr const T& ad(size_t idx) const { assert(idx < size()); return data()[idx]; }

    constexpr T& front() { return (*this)[0]; }
    constexpr const T& front() const { return (*this)[0]; }

    constexpr T& back() { return (*this)[size()-1]; }
    constexpr const T& back() const { return (*this)[size()-1]; }

    constexpr void fill (const T& value)
    {
        for (auto& el : *this) { el = value; }
    }
};

}

#endif // ARRAY_HPP
