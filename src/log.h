/*
    log.h - simple logging

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


#ifndef JOZA_LOG_H
#define JOZA_LOG_H

#include <czmq.h>

extern int g_log_level;

#define ERR(...)                                  \
    do {                                          \
        if (g_log_level >= 1)                       \
            zclock_log("Error: " __VA_ARGS__);    \
    } while(0)
#define WARN(...)                                   \
    do {                                            \
        if (g_log_level >= 2)                         \
            zclock_log("Warning: "  __VA_ARGS__);   \
    } while(0)
#define INFO(...)                               \
    do {                                        \
        if (g_log_level >= 3)                     \
            zclock_log("Info: "  __VA_ARGS__);  \
    } while(0)
#define NOTE(...)                               \
    do {                                        \
        if (g_log_level >= 4)                     \
            zclock_log("Note: "  __VA_ARGS__);  \
    } while(0)
#define TRACE(...)                              \
    do {                                        \
        if (g_log_level >= 5)                     \
            zclock_log("Trace: "  __VA_ARGS__); \
    } while(0)

#endif

