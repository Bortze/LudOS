/*
keyboard.hpp

Copyright (c) 27 Yann BOUCHER (yann)

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
#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include "utils/stdint.h"
#include "../registers.hpp"

#include "kbdmaps.hpp"

#define SCROLL_LOCK_LED 0b001
#define NUM_LOCK_LED 0b010
#define CAPS_LOCK_LED   0b100
#define KBD_PORT 0x60
#define LED_CMD 0xED
#define LED_ACK 0xFA

class Keyboard
{
public:
    struct Event
    {
        bool shift : 1;
        bool alt : 1;
        bool ctrl : 1;
        bool fn : 1;

        bool caps : 1;
        bool scroll : 1;
        bool num : 1;

        bool state : 1;

        // Mapped Key Code
        uint8_t keycode;
    };

public:
    static void init();

    static void set_leds(uint8_t leds);
    static void toggle_led(uint8_t led, bool value = true);

    static void set_kbdmap(const uint8_t* map);

private:
    static void isr(const registers* const);
    static void wait();

public:
    static inline void (*handle_char)(uint8_t); // callback
    static inline void (*kbd_event)(const Event&);

    typedef bool(*key_handler)(const Event&); // if return false : skip handling
    static inline key_handler handlers[256];

    static inline bool lshift { false };
    static inline bool rshift { false };
    static inline bool ctrl   { false };
    static inline bool alt    { false };

    static inline bool caps_lock { false };
    static inline bool scroll_lock { false };
    static inline bool num_lock { false };

    static inline uint8_t leds { 0b000 };

    static inline const uint8_t* kbdmap { kbdmap_us }; // declare kbdmap as pointer to array of int
};

#endif // KEYBOARD_HPP
