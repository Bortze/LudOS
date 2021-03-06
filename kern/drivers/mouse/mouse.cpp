/*
mouse.cpp

Copyright (c) 28 Yann BOUCHER (yann)

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

#include "mouse.hpp"

#include "utils/messagebus.hpp"

#include "halt.hpp"

void Mouse::init()
{
    MessageBus::register_handler<MousePacket>([](const MousePacket& e)
    {
        if (e.x != 0 || e.y != 0)
        {
            MessageBus::send<MouseMoveEvent>({e.x, e.y});
        }

        if (e.wheel != 0)
        {
            MessageBus::send<MouseScrollEvent>({e.wheel});
        }

        if (e.left_button || e.mid_button || e.right_button || e.button_4 || e.button_5)
        {
            MessageBus::send<MouseClickEvent>({e.left_button, e.mid_button, e.right_button, e.button_4, e.button_5});
        }

        left_pressed = e.left_button;
        mid_pressed = e.mid_button;
        right_pressed = e.right_button;
        fourth_pressed = e.button_4;
        fifth_pressed = e.button_5;
    });
}
