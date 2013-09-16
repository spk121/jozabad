/*
    ukey.h - unique key integers

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
#ifndef JOZA_UKEY_H
#define JOZA_UKEY_H

// UKEYs are integer members of a hash key list.
// The list should have the following properties
// - every member is unique
// - the list is strictly monotonically increasing

#include <stdint.h>

#ifndef UKEY_WIDTH
# define UKEY_WIDTH 2
#endif

#if UKEY_WIDTH == 1
typedef uint8_t ukey_t;
#define UKEY_MIN UINT8_C(0)
#define UKEY_MAX UINT8_C(SCHAR_MAX)
#define UKEY_C(x) UINT8_C(x)
#elif UKEY_WIDTH == 2
typedef uint16_t ukey_t;
#define UKEY_MIN UINT16_C(0)
#define UKEY_MAX UINT16_C(INT16_MAX)
#define UKEY_C(x) UINT16_C(x)
#elif UKEY_WIDTH == 4
typedef uint32_t ukey_t;
#define UKEY_MIN UINT32_C(0)
#define UKEY_MAX UINT32_C(INT32_MAX)
#define UKEY_C(x) UINT32_C(x)
#else
# error "Bad UKEY_WIDTH"
#endif

typedef struct {
    size_t index;
    ukey_t key;
} index_ukey_t;

size_t ukey_find(ukey_t arr[], size_t n, ukey_t key);
index_ukey_t ukey_next(ukey_t arr[], size_t n, ukey_t key);

void indexx(ukey_t arr[], size_t n, ukey_t indx[]);
#endif