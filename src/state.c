/*
    state.c

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

#include "state.h"

const char state_names[state_last + 1][STATE_NAME_MAX_LEN + 1] = {
    "READY",
    "X_CALL_REQUEST", "Y_CALL_REQUEST",
    "DATA_TRANSFER",
    "CALL_COLLISION",
    "X_CLEAR_REQUEST", "Y_CLEAR_REQUEST",
    "X_RESET_REQUEST", "Y_RESET_REQUEST"
};

char const *state_name(state_t a)
{
    return state_names[a];
}

