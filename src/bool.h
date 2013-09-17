/*
    bool.h - general categories of errors

    Copyright 2013 Michael L. Gran <spk121@yahoo.com>

    This file is part of Jozabad.

    Jozabad is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Jozabad is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Jozabad.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef JOZA_BOOL_H
#define JOZA_BOOL_H

#include <stdint.h>
// Since this project has a strict policy agains #ifdef, and there
// seems to be no way to use GCC C11 bool (which requires <stdbool.h>)
// as well as Microsoft CL C++11 bool (which doesn't have a <stdbool.h>),
// I just define my own bool type.

// Revist this once Microsoft CL 11.0 C++11 adds <stdbool.h>

typedef int8_t bool_t;
#define TRUE INT8_C(1)
#define FALSE INT8_C(0)

// These statements cause intentional compile errors when I forget to
// use my custom bool type.
#if 0
#pragma GCC diagnostic error "-w"
#define bool _DONT_USE_STD_BOOL;
#define true _DONT_USE_STD_BOOL;
#define false _DONT_USE_STD_BOOL;
#pragma GCC diagnostic pop
#endif

#endif
