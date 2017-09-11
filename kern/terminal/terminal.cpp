/*
TerminalImpl.cpp

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
#include "terminal.hpp"

#include <string.h>
#include <ctype.h>

#ifdef ARCH_i686
#include "i686/pc/devices/speaker.hpp"
#include "i686/pc/serialdebug.hpp"
#include "i686/pc/bios/bda.hpp"
#endif

#include "utils/dynarray.hpp"

#include "io.hpp"
#include "halt.hpp"

#include "utils/minmax.hpp"

TerminalImpl::TerminalImpl(uint16_t* term_buf, size_t iwidth, size_t iheight, size_t imax_history)
    : terminal_buffer(term_buf), width(iwidth), height(iheight), max_history(imax_history),
      history(width*height*max_history)
{
    clear();
}


void TerminalImpl::set_color(uint8_t color)
{
    terminal_color = color;
}


void TerminalImpl::put_entry_at(uint8_t c, uint8_t color, size_t x, size_t y)
{
    check_pos();
    const size_t index = y * width + x;
    terminal_buffer[index] = vga_entry(c, color);
}


void TerminalImpl::put_char(uint8_t c)
{
    if (c == '\n')
    {
        new_line();
    }
    else if (c == '\r')
    {
        terminal_column = 0;
    }
    else if (c == '\b')
    {
        if (terminal_column == 0)
        {
            if (terminal_row > 0)
            {
                //--terminal_row;
            }
        }
        else
        {
            --terminal_column;
        }

        put_entry_at(' ', terminal_color, terminal_column, terminal_row);

        check_pos();
    }
    else if (c == '\t')
    {
        terminal_column += 4;
    }
    else if (c == '\a')
    {
#ifdef ARCH_i686
        Speaker::beep(200);
#endif
    }
    else if (isprint(c))
    {
        put_entry_at(c, terminal_color, terminal_column, terminal_row);
        ++terminal_column;
    }

    check_pos();
}


void TerminalImpl::write(const char *data, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        put_char(data[i]);
    }
}


void TerminalImpl::write_string(const char *data)
{
    write(data, strlen(data));
}


void TerminalImpl::clear()
{
    memsetw(terminal_buffer, vga_entry(' ', terminal_color), height*width*4);
}


void TerminalImpl::scroll_up()
{
    for (size_t y { 1 }; y < height; ++y)
    {
        memcpy(terminal_buffer + (y-1)*width, terminal_buffer + y*width, width*4); // copy line below
    }
    memsetw(terminal_buffer + (height-1)*width, vga_entry(' ', terminal_color), width*4); // clear scrolled line
    --terminal_row;
    update_cursor();
}


void TerminalImpl::push_color(uint8_t color)
{
    //serial::debug::write("%zd\n", history._front);
    old_terminal_color = terminal_color;
    set_color(color);
}


void TerminalImpl::pop_color()
{

    set_color(old_terminal_color);
}


void TerminalImpl::show_history(int page)
{
    if (page < 0)
    {
#ifdef ARCH_i686
        //Speaker::beep(200);
#endif
        page = 0;
    }

    if (static_cast<size_t>(page) > history.size() - height)
    {
#ifdef ARCH_i686
        //Speaker::beep(200);
#endif
        page = history.size() - height; // avoir un plafond, une limite
    }

    current_history_page = page;

    for (size_t i { 0 }; i < height-1; ++i) // ignore first line where everything is typed
    {
        for (size_t j { 0 }; j < width; ++j)
        {
            int index = history.size() - (height-i) -page;
            if (index >= 0)
            {
                //terminal_buffer[i*width+j] = history[index][j];
            }
        }
    }
}


void TerminalImpl::new_line()
{
    add_line_to_history();
    terminal_column = 0;
    ++terminal_row;
}


void TerminalImpl::add_line_to_history()
{
    dynarray<uint16_t> line(width);
    for (size_t i { 0 }; i < width; ++i)
    {
        line[i] = terminal_buffer[terminal_row*width + i];
    }
    //history.add(line);
}


void TerminalImpl::check_pos()
{
    if (terminal_column >= width)
    {
        terminal_column = terminal_column%width;
        add_line_to_history();
        ++terminal_row;
    }
    if (terminal_row >= height)
    {
        scroll_up();
        terminal_row = height-1;
    }

    update_cursor();
}


void TerminalImpl::move_cursor(size_t x, size_t y)
{
#ifdef ARCH_i686
    const size_t index = y * width + x;

    const uint16_t port_low = BDA::video_io_port();
    const uint16_t port_high = port_low + 1;

    // cursor LOW port to vga INDEX register
    outb(port_low, 0x0F);
    outb(port_high, static_cast<uint8_t>(index&0xFF));

    // cursor HIGH port to vga INDEX register
    outb(port_low, 0x0E);
    outb(port_high, static_cast<uint8_t>((index>>8)&0xFF));
#else
#endif
}


void TerminalImpl::update_cursor()
{
    move_cursor(terminal_column, terminal_row);
}

