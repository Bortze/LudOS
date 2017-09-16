/*
kmain.cpp

Copyright (c) 23 Yann BOUCHER (yann)

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

// TODO : FAT32
// TODO : system calls
// TODO : user mode
// TODO : POC calculatrice
// TODO : Paging
// TODO : VFS
// TODO : affichage du nom des exceptions

#ifndef __cplusplus
#error Must be compiler using C++ !
#endif

#ifndef NDEBUG
#define DEBUG
#endif

#include "greet.hpp"

#ifdef ARCH_i686
#include "i686/pc/init.hpp"
#endif

#include "fs/fat.hpp"

#ifdef ARCH_i686
extern "C"
void kmain(uint32_t magic, const multiboot_info_t* mbd_info)
#else
void kmain()
#endif
{
#ifdef ARCH_i686
    i686::pc::init(magic, mbd_info);
#endif

    greet();

    auto fs = fat::read_fat_fs(0);
    if (fs.valid)
    {
        log("FAT %zd filesystem found on drive %zd\n", fs.type, fs.drive);
    }

    while (1)
    {
        nop();
    }
}
