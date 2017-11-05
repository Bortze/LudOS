/*
paging.cpp

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

#include "paging.hpp"

#include <stdint.h>
#include "utils/addr.hpp"

#include "halt.hpp"
#include "panic.hpp"
#include "utils/logging.hpp"

extern "C" int kernel_physical_end;

bitarray<Paging::ram_maxpage, uint32_t> Paging::mem_bitmap;

void Paging::init()
{
#if 0
    uint32_t* pt0 = reinterpret_cast<uint32_t*>(alloc_page_frame());

    uint32_t* pd0 = reinterpret_cast<uint32_t*>(alloc_page_frame());

    pd0[0] = reinterpret_cast<uintptr_t>(pt0);
    pd0[0] |= 3;
    for (size_t i = 1; i < 1024; i++)
    {
        pd0[i] = 0;
    }


    log("%p\n", pt0);

    /* Création de la Page Table[0] */
    uint32_t page_addr = 0;
    for (size_t i = 0; i < 1024; i++)
    {
        pt0[i] = page_addr;
        pt0[i] |= 3;
        page_addr += 0x1000;
    }
    // must use inline assembly, cannot call external function as that would mess up the stack
    asm("   mov %0, %%eax    \n \
    mov %%eax, %%cr3 \n \
            mov %%cr0, %%eax \n \
            or $0x80000001, %%eax     \n \
            mov %%eax, %%cr0" :: "m"(pd0));

            log("Paging initialized\n");
#endif
}

uintptr_t Paging::alloc_page_frame(size_t number)
{
    size_t counter { 0 };
    size_t page_addr { 0 };
    for (size_t i { 0 }; i < mem_bitmap.array_size; ++i)
    {
        if (!mem_bitmap[i])
        {
            ++counter;
            if (counter == 1)
            {
                page_addr = i;
            }
        }

        if (counter >= number)
        {
            for (size_t i { page_addr }; i < page_addr+counter; ++i)
            {
                mem_bitmap[i] = true;
            }
            return phys(page_addr*page_size);
        }
    }
    return 0;
}

bool Paging::release_page_frame(uintptr_t p_addr, size_t number)
{
    if (p_addr < reinterpret_cast<uintptr_t>(&kernel_physical_end))
    {
        panic("Should not happend ! %p\n", p_addr);
    }

    size_t base_page = page(virt(p_addr));
    bool released = false;

    for (size_t i { 0 }; i < number; ++i)
    {
        released |= mem_bitmap[base_page+i];
        mem_bitmap[base_page+i] = false;
    }

    return released;
}

// TODO : implement
void Paging::map_page(void *phys, void *&virt, uint32_t flags)
{
    virt = phys;
}

void Paging::mark_as_used(uintptr_t addr, size_t size)
{
    for (size_t i { page(addr) }; i < page(addr + size); ++i)
    {
        mem_bitmap[i] = true;
    }
}
