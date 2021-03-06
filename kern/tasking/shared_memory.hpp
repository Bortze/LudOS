/*
shared_memory.hpp

Copyright (c) 05 Yann BOUCHER (yann)

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
#ifndef SHARED_MEMORY_HPP
#define SHARED_MEMORY_HPP

#include <stdint.h>
#include <vector.hpp>
#include <memory.hpp>

#include "utils/gsl/gsl_span.hpp"
#include "mem/memmap.hpp"
#include "sys/types.h"

class SharedMemorySegment
{
public:
    SharedMemorySegment(size_t size_in_pages);
    ~SharedMemorySegment();

public:
    void map(void* v_addr, uint32_t flags = Memory::Read|Memory::Write|Memory::User);
    void unmap(void* v_addr);

    size_t size() const;

private:
    std::vector<uintptr_t> m_phys_addrs;
};

unsigned int create_shared_memory_id();
std::shared_ptr<SharedMemorySegment> create_shared_mem(unsigned int id, size_t size);
std::shared_ptr<SharedMemorySegment> get_shared_mem(unsigned int id);

#endif // SHARED_MEMORY_HPP
