/*
liballoc_stubs.cpp

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

#include "spinlock.hpp"

#include "i686/pc/paging.hpp"

#include <stdint.h>

DECLARE_LOCK(liballoc_lock);

extern "C"
{
int liballoc_lock()
{
    LOCK(liballoc_lock);
    return 0;
}

int liballoc_unlock()
{
    UNLOCK(liballoc_lock);
    return 0;
}

void* liballoc_alloc(size_t pages)
{
    //return Paging::alloc_page_frame(pages);
}

int liballoc_free(void* ptr, size_t pages)
{
    //return Paging::release_page_frame((uintptr_t)ptr, pages);
}
}
