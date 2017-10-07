/*
diskinterface.hpp

Copyright (c) 16 Yann BOUCHER (yann)

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
#ifndef DISKINTERFACE_HPP
#define DISKINTERFACE_HPP

#include <stdint.h>

#include <functional.hpp>

class DiskInterface
{
public:
    enum class Error
    {
        OK,
        Unknown
    };

public:
    static inline bool read(size_t disk_num, uint32_t sector, uint8_t count, uint8_t* buf)
    {
        return read_impl(disk_num, sector, count, buf);
    }

    static inline bool write(size_t disk_num, uint32_t sector, uint8_t count, const uint8_t* buf)
    {
        return write_impl(disk_num, sector, count, buf);
    }

    static inline std::vector<uint32_t> scan()
    {
        return scan_impl();
    }

    static inline std::function<bool(size_t disk_num, uint32_t sector, uint8_t count, uint8_t* buf)> read_impl;
    static inline std::function<bool(size_t disk_num, uint32_t sector, uint8_t count, const uint8_t* buf)> write_impl;
    static inline std::function<std::vector<uint32_t>()> scan_impl;

    static inline Error last_error { Error::OK };
};

#endif // DISKINTERFACE_HPP
