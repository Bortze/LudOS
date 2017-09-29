/*
terminal.hpp

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
#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include <stdint.h>
#include "utils/addr.hpp"
#include "stack.hpp"
#include "graphics/color.hpp"

#include <functional.hpp>

#include "historybuffer.hpp"

class Terminal
{
public:
    Terminal(size_t iwidth, size_t iheight, size_t imax_history = 5);

public:
    void put_entry_at(uint8_t c, TermEntry color, size_t x, size_t y);
    void put_char(uint8_t c);
    void write(const char* data, size_t size);
    void write_string(const char* data);
    void clear();
    void scroll_up();
    void push_color(TermEntry color);
    void pop_color();
    void show_history(int page);
    uint8_t current_history() const { return current_history_page; }
    void scroll_history(int scroll) { show_history(current_history()+scroll); }
    TermEntry color() const;

    size_t width() const { return _width; }
    size_t height() const { return _height; }

private:
    void new_line();
    void add_line_to_history();
    void check_pos();
    void update_cursor();

public:
    std::function<void(size_t x, size_t y, size_t width)> move_cursor_callback;
    std::function<void(size_t ms)> beep_callback;
    std::function<void(size_t x, size_t y, uint8_t c, TermEntry color)> putchar_callback;

private:
    size_t terminal_row { 0 };
    size_t terminal_column { 0 };
    uint16_t* terminal_buffer { reinterpret_cast<uint16_t*>(phys(0xB8000)) };

    std::stack<TermEntry> color_stack;

    const size_t _width;
    const size_t _height;
    const size_t max_history;

    HistoryBuffer history;
    uint8_t current_history_page { 0 };
};

extern Terminal* term;

#endif // TERMINAL_HPP
