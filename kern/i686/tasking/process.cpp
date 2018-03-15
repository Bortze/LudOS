/*
process.cpp

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

#include "tasking/process.hpp"
#include "i686/tasking/process.hpp"

#include <string.hpp>
#include <vector.hpp>

#include "i686/gdt/gdt.hpp"
#include "i686/interrupts/interrupts.hpp"

#include "utils/membuffer.hpp"
#include "utils/memutils.hpp"

extern "C" void enter_ring3(uint32_t esp, uint32_t eip);

constexpr size_t user_stack_top = KERNEL_VIRTUAL_BASE - sizeof(uintptr_t);

void map_code(const Process::ArchSpecificData& data)
{
    size_t code_page_amnt = data.code.size() / Paging::page_size +
            (data.code.size()%Paging::page_size?1:0);

    for (size_t i { 0 }; i < code_page_amnt; ++i)
    {
        uint8_t* phys_addr = (uint8_t*)Paging::physical_address((uint8_t*)data.code.data() + i*Paging::page_size);
        uint8_t* virt_addr = (uint8_t*)(i * Paging::page_size);

        assert(phys_addr);
        Paging::map_page(phys_addr, virt_addr, Memory::Read|Memory::Write|Memory::User);
    }
}

void map_stack(const Process::ArchSpecificData& data)
{
    size_t code_page_amnt = data.stack.size() / Paging::page_size +
            (data.stack.size()%Paging::page_size?1:0);

    for (size_t i { 0 }; i < code_page_amnt; ++i)
    {
        uint8_t* phys_addr = (uint8_t*)Paging::physical_address((uint8_t*)data.stack.data() + i*Paging::page_size);
        uint8_t* virt_addr = (uint8_t*)(user_stack_top - (i * Paging::page_size));

        assert(phys_addr);
        Paging::map_page(phys_addr, virt_addr, Memory::Read|Memory::Write|Memory::NoExec|Memory::User);
    }
}

void unmap_code(const Process::ArchSpecificData& data)
{
    size_t code_page_amnt = data.code.size() / Paging::page_size +
            (data.code.size()%Paging::page_size?1:0);

    for (size_t i { 0 }; i < code_page_amnt; ++i)
    {
        uint8_t* virt_addr = (uint8_t*)(i * Paging::page_size);

        Paging::unmap_page(virt_addr);
    }
}

void unmap_stack(const Process::ArchSpecificData& data)
{
    size_t code_page_amnt = data.stack.size() / Paging::page_size +
            (data.stack.size()%Paging::page_size?1:0);

    for (size_t i { 0 }; i < code_page_amnt; ++i)
    {
        uint8_t* virt_addr = (uint8_t*)(user_stack_top - (i * Paging::page_size));

        Paging::unmap_page(virt_addr);
    }
}

Process::~Process()
{
    stop();

    delete arch_data;
}

Process &Process::current()
{
    assert(m_current_process);

    return *m_current_process;
}

void Process::execute()
{
    map_code(*arch_data);
    map_stack(*arch_data);

    m_current_process = this;

    enter_ring3(user_stack_top, 0x0);
}

void Process::stop()
{
    unmap_code(*arch_data);
    unmap_stack(*arch_data);
}

void Process::arch_init(gsl::span<const uint8_t> code_to_copy)
{
    arch_data = new ArchSpecificData;

    arch_data->stack.resize(Paging::page_size);
    arch_data->code.resize(code_to_copy.size());
    std::copy(code_to_copy.begin(), code_to_copy.end(), arch_data->code.begin());
}
