/*
driver.hpp

Copyright (c) 15 Yann BOUCHER (yann)

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
#ifndef NETDRIVER_HPP
#define NETDRIVER_HPP

#include "drivers/driver.hpp"

#include <array.hpp>
#include <stdexcept.hpp>
#include <string.hpp>

#include "utils/vecutils.hpp"

// TODO : Use the messagebus for sending/receiving packets

class NetworkException : public std::runtime_error
{
public:
    enum ErrorType
    {
        NoNicFound,
        Unkown
    };

    std::string to_string(ErrorType type)
    {
        switch (type)
        {
            case NoNicFound:
                return "No network controller found";
            default:
                return "Unknown error";
        }
    }

    explicit NetworkException(ErrorType type)
        : std::runtime_error("Network error : " + to_string(type))
    {

    }
};

class NetworkDriver : public Driver
{
public:
    static NetworkDriver& get();

    virtual std::array<uint8_t, 6> mac_address() const = 0;

protected:
    static void add_nic(NetworkDriver& nic);

private:
    static inline ref_vector<NetworkDriver> m_nics;
};

#endif // NETDRIVER_HPP
