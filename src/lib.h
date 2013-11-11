/*
    lib.h - miscellaneous library functions

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
#ifndef JOZA_LIB_H
#define JOZA_LIB_H

#include <glib.h>
#include <stdint.h>
#include <stdlib.h>


//----------------------------------------------------------------------------
// BASICS
//----------------------------------------------------------------------------

size_t intlen(int x);
char *cstrdup (const char *s);

#if __GLIBC__ == 2
// This can be removed once Annex K hits GNU libc
size_t strnlen_s (const char *s, size_t maxsize);
#endif

//----------------------------------------------------------------------------
// ASCII-STYLE NAMES
//----------------------------------------------------------------------------
gboolean safeascii(const char *mem, size_t n);

//----------------------------------------------------------------------------
// PHONE-STYLE NAMES
//----------------------------------------------------------------------------
gboolean safe121 (const char *str, size_t n);
uint32_t pack121(const char *str, size_t n);
const char *unpack121(uint32_t x);

//----------------------------------------------------------------------------
// INDEX SORTING OF STRINGS
//----------------------------------------------------------------------------

// void qisort(char *arr[], size_t n, size_t indx[]);
// size_t namefind(const char *arr[], size_t n, size_t nidx[], const char *str);

//----------------------------------------------------------------------------
// SEARCHING ORDERED UINT32_T ARRAYS
//----------------------------------------------------------------------------

// size_t ifind(uint32_t arr[], size_t n, uint32_t X);

//----------------------------------------------------------------------------
// STRICT C11 TIME
//----------------------------------------------------------------------------

double now(void);

#endif  /* LIB_H */

