#ifndef JOZA_MSG_H
#define JOZA_MSG_H

#define RESET_REQUEST(A,C,D) \
    joza_msg_send_addr_reset_request(g_poll_sock, (A), (C), (D));
#define CLEAR_REQUEST(A,C,D) \
    joza_msg_send_addr_reset_request(g_poll_sock, (A), (C), (D));
#define DIAGNOSTIC(A,C,D) \
    joza_msg_send_addr_reset_request(g_poll_sock, (A), (C), (D))

#endif
