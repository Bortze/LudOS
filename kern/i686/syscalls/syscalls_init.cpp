/*
syscalls_init.cpp

Copyright (c) 11 Yann BOUCHER (yann)

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

#include "i686/syscalls/syscall.hpp"
#include "syscalls/LudOS/syscalls.hpp"
#include "syscalls/Linux/syscalls.hpp"

#include <type_traits.hpp>
#include <functional.hpp>

#include <errno.h>

#include "i686/cpu/registers.hpp"

#include "syscalls_init.tpp"

int invalid_syscall(const registers* const r)
{
    warn("Invalid syscall number : %d\n", r->eax);
    return ENOSYS;
}

int test(int a, char b, uint64_t qword, uint16_t word, uint32_t dword, uint8_t almost_too_big)
{
    log_serial("%d %c\n", a, b);
    return EOK;
}

void init_syscalls()
{
    using namespace SyscallType;

    // init table with correct values
    for (size_t i { 0 }; i < max_syscalls; ++i)
    {
        linux_syscall_table[i].ptr = ludos_syscall_table[i].ptr = invalid_syscall;
        linux_syscall_table[i].arg_cnt = ludos_syscall_table[i].arg_cnt = invalid_syscall_magic;
    }

    ////// LudOS

    add_syscall<LudOS, max_syscalls-1>(test);

    add_syscall<LudOS, 0>(sys_print_serial);
    add_syscall<LudOS, 1>(sys_print_debug);
    add_syscall<LudOS, 2>(sys_panic);


    ////// Linux
    add_syscall<Linux, 183>(sys_getcwd);

}
