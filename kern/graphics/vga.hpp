/*
vga.hpp

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
#ifndef VGA_HPP
#define VGA_HPP

#include <stdint.h>

#include <vector.hpp>

#include "color.hpp"

#include "utils/logging.hpp"
#include "utils/stlutils.hpp"

namespace graphics
{

namespace vga
{

/* Hardware text mode color constants. */
enum color : uint8_t
{
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_YELLOW = 14,
    VGA_COLOR_WHITE = 15,
};

static inline uint8_t entry_color(color fg,  color bg)
{
    return fg | bg << 4;
}

static inline uint16_t entry(uint8_t uc, uint8_t color)
{
    return static_cast<uint16_t>(uc) | static_cast<uint16_t>(color) << 8;
}

static inline color color_to_vga(const Color& color)
{
    std::vector<std::pair<Color, vga::color>> colors
    {
        {0x000000, VGA_COLOR_BLACK},
        {0x0000aa, VGA_COLOR_BLUE},
        {0x00aa00, VGA_COLOR_GREEN},
        {0x00aaaa, VGA_COLOR_CYAN},
        {0xaa0000, VGA_COLOR_RED},
        {0xaa00aa, VGA_COLOR_MAGENTA},
        {0xaa5500, VGA_COLOR_BROWN},
        {0xaaaaaa, VGA_COLOR_LIGHT_GREY},
        {0x555555, VGA_COLOR_DARK_GREY},
        {0x5555ff, VGA_COLOR_LIGHT_BLUE},
        {0x55ff55, VGA_COLOR_LIGHT_GREEN},
        {0x55ffff, VGA_COLOR_LIGHT_CYAN},
        {0xff5555, VGA_COLOR_LIGHT_RED},
        {0xff55ff, VGA_COLOR_LIGHT_MAGENTA},
        {0xffff55, VGA_COLOR_YELLOW},
        {0xffffff, VGA_COLOR_WHITE}
    };

    using color_pair = std::pair<Color, vga::color>;

    return closest<color_pair,size_t>({color, VGA_COLOR_BLACK}, colors, [](const color_pair& lhs, const color_pair& rhs)
    {
        return (rhs.first.r - lhs.first.r)*(rhs.first.r - lhs.first.r) +
               (rhs.first.g - lhs.first.g)*(rhs.first.g - lhs.first.g) +
               (rhs.first.b - lhs.first.b)*(rhs.first.b - lhs.first.b);
    }).second;
}

}

}

#endif // VGA_HPP
