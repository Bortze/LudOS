/*
memmap.hpp

Copyright (c) 09 Yann BOUCHER (yann)

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
#ifndef MEMMAP_HPP
#define MEMMAP_HPP

#include <stdint.h>
#include <string.h>
#include <limits.h>

class VM
{
public:
    enum MmapFlags : uint32_t
    {
        Read         = 1<<0,
        Write        = 1<<1,
        User         = 1<<2,
        Uncached     = 1<<3,
        WriteThrough = 1<<4,
        NoExec       = 1<<5
    };

    static void* mmap(uintptr_t p_addr, size_t len, uint32_t flags = Read|Write);
    static void unmap(void* v_addr, size_t len);

    static void map_page(uintptr_t p_addr, void* v_addr, uint32_t flags = VM::Read|VM::Write);
    static void unmap_page(void* v_addr);

    static bool is_mapped(const void* v_addr);

    static bool check_user_ptr(const void *v_addr, size_t size);

    static uintptr_t physical_address(const void* v_addr);

    static uintptr_t allocate_physical_page();
    static void release_physical_page(uintptr_t page);

    static size_t page_size();

    static inline uintptr_t page(uintptr_t addr)
    {
        return (addr & ~(page_size()-1));
    }

    static inline uintptr_t offset(uintptr_t addr)
    {
        return addr & (page_size()-1);
    }

    static void phys_read(uintptr_t addr, void* buf, size_t size)
    {
        auto ptr = VM::mmap(addr, size, VM::Read);
        memcpy(buf, ptr, size);
        VM::unmap(ptr, size);
    }

    static void phys_write(uintptr_t addr, const void* buf, size_t size)
    {
        auto ptr = VM::mmap(addr, size, VM::Write);
        memcpy(ptr, buf, size);
        VM::unmap(ptr, size);
    }
};

#endif // MEMMAP_HPP
