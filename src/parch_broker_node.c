

//  ---------------------------------------------------------------------
//  Context for each client connection

struct _parch_broker_nodet_t {
    //  Properties accessible to client actions
    int heartbeat;              //  Client heartbeat interval
    event_t next_event;         //  Next event
    // BEGIN CLIENT CONTEXT                                                               
    void *workerpipe;        // Socket to talk to selected sub-server                     
                                                                                          
    uint64_t timer_t12;                                                                   
    int timeout_count_t12;                                                                
    uint64_t timer_t13;                                                                   
    int timeout_count_t13;                                                                
                                                                                          
    int32_t sent;           // sequence number of the last data packet sent               
    int32_t authorized;     // lower limits of client's allowed packet window             
    int64_t window;         // size of client's allowed packet window                     
    int32_t ready;          // true/false - false when client has send a RECEIVE_NOT_READY
                                                                                          
    int32_t received;       // sequence number of last data packet received from client   
    // END CLIENT CONTEXT                                                                 
    //  Properties you should NOT touch
    void *router;               //  Socket to client
    int64_t heartbeat_at;       //  Next heartbeat at this time
    int64_t expires_at;         //  Expires at this time
    state_t state;              //  Current state
    event_t event;              //  Current event
    char *hashkey;              //  Key into clients hash
    zframe_t *address;          //  Client address identity
    x25_msg_t *request;         //  Last received request
    x25_msg_t *reply;           //  Reply to send out, if any
};
