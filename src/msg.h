/*
    msg.h - helper functions for ZeroMQ activities

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
#ifndef JOZA_MSG_H
#define JOZA_MSG_H

#define RESET_REQUEST(A,C,D) \
    joza_msg_send_addr_reset_request(g_poll_sock, (A), (C), (D))
#define CLEAR_REQUEST(A,C,D) \
    joza_msg_send_addr_reset_request(g_poll_sock, (A), (C), (D))
#define DIAGNOSTIC(A,C,D) \
    joza_msg_send_addr_reset_request(g_poll_sock, (A), (C), (D))
#define CONNECT_INDICATION(A) \
    joza_msg_send_addr_connect_indication(g_poll_sock, (A))
uint32_t msg_addr2hash (const zframe_t *z);

#endif
