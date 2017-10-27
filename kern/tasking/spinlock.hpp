/*
spinlock.hpp

Copyright (c) 31 Yann BOUCHER (yann)

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
#ifndef SPINLOCK_HPP
#define SPINLOCK_HPP

typedef volatile int spinlock_t;

#define DECLARE_LOCK(name) spinlock_t name ## Locked
#define LOCK(name) \
        while (!__sync_bool_compare_and_swap(& name ## Locked, 0, 1)); \
        __sync_synchronize();
#define UNLOCK(name) \
        __sync_synchronize(); \
        name ## Locked = 0;

#define LOCK_VAL(name) \
        while (!__sync_bool_compare_and_swap(name, 0, 1)); \
        __sync_synchronize();
#define UNLOCK_VAL(name) \
        __sync_synchronize(); \
        *name = 0;

#endif // SPINLOCK_HPP
