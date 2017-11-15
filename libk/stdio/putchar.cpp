/*
putchar.c

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

#include <stdio.h>

#ifdef __is_libk
#include <terminal/terminal.hpp>
#include <i686/pc/serial/serialdebug.hpp>
#endif

#include "unicode/utf8decoder.hpp"

bool putc_serial = false;

UTF8Decoder decoder;

void putchar(char c)
{
#ifdef __is_libk
    decoder.feed(c);
    if (decoder.ready())
    {
        putcharw(decoder.spit());
    }
#else
    // TODO : do !
#error Not implemented yet
#endif
}

void putcharw(char32_t c)
{
#ifdef __is_libk
        term().put_char(c);
        if (putc_serial) serial::debug::write("%c", c);
#else
    // TODO : do !
#error Not implemented yet
#endif
}
