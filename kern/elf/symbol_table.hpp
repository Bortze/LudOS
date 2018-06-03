/*
symbol_table.hpp

Copyright (c) 31 Yann BOUCHER (yann)

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
#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <stdint.h>

#include <unordered_map.hpp>
#include <optional.hpp>

#include <kstring/kstring.hpp>

#include <utils/gsl/gsl_span.hpp>

#include "elf.hpp"

namespace elf
{

struct SymbolInfo
{
    kpp::string name;
    kpp::string file;
    uintptr_t offset;
};

struct SymbolTable
{
    kpp::optional<SymbolInfo> get_function(uintptr_t addr)
    {
        while (addr-- > KERNEL_VIRTUAL_BASE)
        {
            if (auto it = table.find(addr); it != table.end())
            {
                return it->second;
            }
        }

        return {};
    }

    std::unordered_map<uintptr_t, SymbolInfo> table;
};

bool has_symbol_table(const Elf32_Shdr* base, size_t sh_num);

SymbolTable get_symbol_table(const Elf32_Shdr* base, size_t sh_num);

SymbolTable get_symbol_table_file(gsl::span<const uint8_t> file);

extern SymbolTable kernel_symbol_table;

}

#endif // SYMBOL_TABLE_HPP
