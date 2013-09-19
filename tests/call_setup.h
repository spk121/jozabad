#ifndef JOZA_TESTS_CALL_SETUP_H
#define JOZA_TESTS_CALL_SETUP_H
static void call_setup (int verbose, void *sock1, void *sock2, char *name1, char *name2,
                        packet_t packet, uint16_t window, tput_t thru)
{
    int ret;
       
    if (verbose)
        printf("peer X: requesting a virtual call\n");
    ret = joza_msg_send_call_request(sock1, name1, name2,
                                     packet, window, thru, zframe_new (0,0));

    if (verbose)
        printf("peer Y: waiting for a call request message\n");
    joza_msg_t *response = joza_msg_recv(sock2);
    if (joza_msg_id(response) != JOZA_MSG_CALL_REQUEST) {
        if (verbose) {
            printf("peer Y: did not receive call request from X\n");
            joza_msg_dump(response);
        }
        exit (1);
    }   

    if (verbose)
        printf("peer Y: sending call accepted message\n");
    joza_msg_send_call_accepted(sock2, name1, name2,
                                packet, window, thru, zframe_new(0,0));

    joza_msg_destroy(&response);
    
    if (verbose)
        printf("peer X: waiting for a call accepted message\n");
    response = joza_msg_recv(sock1);
    if (joza_msg_id(response) != JOZA_MSG_CALL_ACCEPTED) {
        if (verbose) {
            printf("peer X: did not receive call request from Y\n");
            joza_msg_dump(response);
        }
        exit (1);
    } 
}
#endif
