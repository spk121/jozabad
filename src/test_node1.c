//
//  Majordomo Protocol worker example

#include "../include/parch.h"

int main(int argc, char *argv []) {
    // int verbose = (argc > 1 && streq (argv [1], "-v"));
    int verbose = 1;
    parch_node_t *session = parch_node_new(
            "tcp://localhost:5555", "echo", verbose);

    if (argc > 1 && streq(argv[1], "C1")) {
        parch_msg_t *request;

        // This is the client side of test 1
        parch_msg_send_connect(parch_node_client(session),
                (char *) parch_node_service(session),
                zframe_new(NULL, 0));

        parch_msg_send_call_request(parch_node_client(session), (char *) "echo");

        // Wait for a call acceptance
        request = parch_msg_recv(parch_node_client(session));
        assert(request);
        parch_msg_dump(request);
        assert(parch_msg_id(request) == PARCH_MSG_CALL_ACCEPTED);
        parch_msg_destroy(&request);

        // Respond with a data packet
        parch_msg_send_data(parch_node_client(session), 0, zframe_new("DATA FOO", 8));

        // Receive a data packet in response
        request = parch_msg_recv(parch_node_client(session));
        assert(request);
        parch_msg_dump(request);
        assert(parch_msg_id(request) == PARCH_MSG_DATA);
        parch_msg_destroy(&request);


        // Clear the channel
        parch_msg_send_clear_request(parch_node_client(session), 0, 0);

        // Wait for receive a clear channel confirmation
        request = parch_msg_recv(parch_node_client(session));
        assert(request);
        parch_msg_dump(request);
        assert(parch_msg_id(request) == PARCH_MSG_CLEAR_CONFIRMATION);
        parch_msg_destroy(&request);

        // Disconnect
        parch_msg_send_disconnect(parch_node_client(session));

    } else if (argc > 1 && streq(argv[1], "S1")) {
        // This is the server side of Test 1
        parch_msg_t *request;

        // Connect to the broker
        parch_msg_send_connect(parch_node_client(session),
                (char *) parch_node_service(session),
                zframe_new(NULL, 0));

        // Wait for a call request on my service
        request = parch_msg_recv(parch_node_client(session));
        assert(request);
        parch_msg_dump(request);
        assert(parch_msg_id(request) == PARCH_MSG_CALL_REQUEST);
        assert(streq(parch_msg_service_requested(request), "echo"));
        parch_msg_destroy(&request);

        // Respond that the call is accepted
        parch_msg_send_call_accepted(parch_node_client(session));

        // Wait for a data packet
        request = parch_msg_recv(parch_node_client(session));
        assert(request);
        parch_msg_dump(request);
        assert(parch_msg_id(request) == PARCH_MSG_DATA);
        parch_msg_destroy(&request);

        // Respond with a data packet
        parch_msg_send_data(parch_node_client(session), 0, zframe_new("DATA BAR", 8));

        // Wait for receive a clear channel request
        request = parch_msg_recv(parch_node_client(session));
        assert(request);
        parch_msg_dump(request);
        assert(parch_msg_id(request) == PARCH_MSG_CLEAR_REQUEST);
        parch_msg_destroy(&request);

        // Respond with a clear confirmation
        parch_msg_send_clear_confirmation(parch_node_client(session));

        // Disconnect

    } else if (argc > 1 && streq(argv[1], "C2")) {
        // This is the client side of test 2
        parch_msg_send_connect(parch_node_client(session),
                (char *) parch_node_service(session),
                zframe_new(NULL, 0));
        parch_msg_send_rnr(parch_node_client(session));
    }

    parch_msg_t *msg = parch_msg_recv(parch_node_client(session));
    parch_msg_dump(msg);
    parch_node_destroy(&session);
    return 0;
}
