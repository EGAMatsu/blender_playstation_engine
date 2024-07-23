/*

MIT License

Copyright (c) 2022 PCSX-Redux authors

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

#pragma once

#include <stdint.h>

#include "psyqo/primitives/common.hh"

// This file holds all of the constants used throughout the example.

static constexpr psyqo::Color HIRED = {{.r = 240, .g = 0, .b = 0}};
static constexpr psyqo::Color HIORANGE = {{.r = 240, .g = 160, .b = 0}};
static constexpr psyqo::Color HIYELLOW = {{.r = 240, .g = 240, .b = 0}};
static constexpr psyqo::Color HIGREEN = {{.r = 0, .g = 240, .b = 0}};
static constexpr psyqo::Color HIBLUE = {{.r = 0, .g = 0, .b = 240}};
static constexpr psyqo::Color HICYAN = {{.r = 0, .g = 240, .b = 240}};
static constexpr psyqo::Color HIPURPLE = {{.r = 160, .g = 0, .b = 240}};

static constexpr psyqo::Color RED = {{.r = 180, .g = 82, .b = 84}};
static constexpr psyqo::Color ORANGE = {{.r = 210, .g = 160, .b = 104}};
static constexpr psyqo::Color YELLOW = {{.r = 236, .g = 224, .b = 158}};
static constexpr psyqo::Color GREEN = {{.r = 176, .g = 176, .b = 98}};
static constexpr psyqo::Color BLUE = {{.r = 76, .g = 128, .b = 202}};
static constexpr psyqo::Color CYAN = {{.r = 102, .g = 194, .b = 210}};
static constexpr psyqo::Color PURPLE = {{.r = 206, .g = 176, .b = 202}};

static constexpr psyqo::Color WHITE = {{.r = 255, .g = 255, .b = 255}};
static constexpr psyqo::Color GREY = {{.r = 160, .g = 160, .b = 160}};
static constexpr psyqo::Color DARKGREY = {{.r = 80, .g = 80, .b = 80}};
static constexpr psyqo::Color DARKERGREY = {{.r = 40, .g = 40, .b = 40}};
static constexpr psyqo::Color BLACK = {{.r = 0, .g = 0, .b = 0}};

static constexpr psyqo::Color PIECES_COLORS[7] = {CYAN, YELLOW, PURPLE, BLUE, ORANGE, RED, GREEN};
static constexpr psyqo::Color PIECES_HICOLORS[7] = {HICYAN, HIYELLOW, HIPURPLE, HIBLUE, HIORANGE, HIRED, HIGREEN};
