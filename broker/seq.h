/*
    seq.h - numeric type for sequence numbers

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

#ifndef JOZA_SEQ_H
#define	JOZA_SEQ_H

#ifndef SEQ_WIDTH
# define SEQ_WIDTH 2
#endif

#if SEQ_WIDTH == 1
typedef uint8_t seq_t;
typedef uint16_t dseq_t;
#define SEQ_MIN UINT8_C(0)
#define SEQ_MAX UINT8_C(SCHAR_MAX)
#define SEQ_C(x) UINT8_C(x)
#elif SEQ_WIDTH == 2
typedef uint16_t seq_t;
typedef uint32_t dseq_t;
#define SEQ_MIN UINT16_C(0)
#define SEQ_MAX UINT16_C(INT16_MAX)
#define SEQ_C(x) UINT16_C(x)
#elif SEQ_WIDTH == 4
typedef uint32_t seq_t;
typedef uint64_t dseq_t;
#define SEQ_MIN UINT32_C(0)
#define SEQ_MAX UINT32_C(INT32_MAX)
#define SEQ_C(x) UINT32_C(x)
#else
# error "Bad SEQ_WIDTH"
#endif

#endif
