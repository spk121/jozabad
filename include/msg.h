/*  =========================================================================
    msg - transport for switched virtual call messages

    Generated codec header for msg
    -------------------------------------------------------------------------
    GPL3+, bitches
    =========================================================================
*/

#ifndef __MSG_H_INCLUDED__
#define __MSG_H_INCLUDED__

/*  These are the msg messages
    DATA - Binary data message.
        sequence      number 4
        data          frame
    RR - Tells peer the lowest send sequence number that it may send in its DATA packet.
        sequence      number 4
    RNR - Tells peer to stop sending data.
        sequence      number 4
    CALL_REQUEST - Call a peer. Negotiate the type of connection requested.
        service       string
        directionality  number 1
        packet        number 1
        window        number 2
        throughput    number 1
    CALL_ACCEPTED - Answer the call.
        directionality  number 1
        packet        number 1
        window        number 2
        throughput    number 1
    CLEAR_REQUEST - Request call termination
        cause         number 1
        diagnostic    number 1
    CLEAR_CONFIRMATION - Accept call termination
        cause         number 1
        diagnostic    number 1
    RESET_REQUEST - Tell the peer to restart flow control
        cause         number 1
        diagnostic    number 1
    RESET_CONFIRMATION - Tell the peer that we have restarted flow control
        cause         number 1
        diagnostic    number 1
    CONNECT - Client node requests connection to the broker.
        protocol      string
        version       number 1
        service       string
        directionality  number 1
        throughput    number 1
    CONNECT_INDICATION - Broker tells node that it has been connected.
        directionality  number 1
        throughput    number 1
    DISCONNECT - Node tells broker that it is disconnecting.
    DISCONNECT_INDICATION - Broker tells node that it has been disconnect
        cause         number 1
        diagnostic    number 1
    COUNT - This is the Switched Virtual Call protocol version 1
*/

#include <cstdint>

#define MSG_VERSION                         1

#define MSG_DATA                            0
#define MSG_RR                              1
#define MSG_RNR                             2
#define MSG_CALL_REQUEST                    3
#define MSG_CALL_ACCEPTED                   4
#define MSG_CLEAR_REQUEST                   5
#define MSG_CLEAR_CONFIRMATION              6
#define MSG_RESET_REQUEST                   7
#define MSG_RESET_CONFIRMATION              8
#define MSG_CONNECT                         9
#define MSG_CONNECT_INDICATION              10
#define MSG_DISCONNECT                      11
#define MSG_DISCONNECT_INDICATION           12
#define MSG_COUNT                           13

//  Structure of our class

struct _msg_t {
    zframe_t *address;          //  Address of peer if any
    int id;                     //  msg message ID
    byte *needle;               //  Read/write pointer for serialization
    byte *ceiling;              //  Valid upper limit for read pointer
    uint32_t sequence;
    zframe_t *data;
    char *service;
    byte directionality;
    byte packet;
    uint16_t window;
    byte throughput;
    byte cause;
    byte diagnostic;
    char *protocol;
    byte version;
};

//  Opaque class structure
typedef struct _msg_t msg_t;

//  @interface
//  Create a new msg
msg_t *
    msg_new (int id);

//  Destroy the msg
void
    msg_destroy (msg_t **self_p);

//  Receive and parse a msg from the input
msg_t *
    msg_recv (void *input);

//  Send the msg to the output, and destroy it
int
    msg_send (msg_t **self_p, void *output);

//  Send the DATA to the output in one step
int
    msg_send_data (void *output,
        uint32_t sequence,
        zframe_t *data);

//  Send the RR to the output in one step
int
    msg_send_rr (void *output,
        uint32_t sequence);

//  Send the RNR to the output in one step
int
    msg_send_rnr (void *output,
        uint32_t sequence);

//  Send the CALL_REQUEST to the output in one step
int
    msg_send_call_request (void *output,
        char *service,
        byte directionality,
        byte packet,
        uint16_t window,
        byte throughput);

//  Send the CALL_ACCEPTED to the output in one step
int
    msg_send_call_accepted (void *output,
        byte directionality,
        byte packet,
        uint16_t window,
        byte throughput);

//  Send the CLEAR_REQUEST to the output in one step
int
    msg_send_clear_request (void *output,
        byte cause,
        byte diagnostic);

//  Send the CLEAR_CONFIRMATION to the output in one step
int
    msg_send_clear_confirmation (void *output,
        byte cause,
        byte diagnostic);

//  Send the RESET_REQUEST to the output in one step
int
    msg_send_reset_request (void *output,
        byte cause,
        byte diagnostic);

//  Send the RESET_CONFIRMATION to the output in one step
int
    msg_send_reset_confirmation (void *output,
        byte cause,
        byte diagnostic);

//  Send the CONNECT to the output in one step
int
    msg_send_connect (void *output,
        char *service,
        byte directionality,
        byte throughput);

//  Send the CONNECT_INDICATION to the output in one step
int
    msg_send_connect_indication (void *output,
        byte directionality,
        byte throughput);

//  Send the DISCONNECT to the output in one step
int
    msg_send_disconnect (void *output);

//  Send the DISCONNECT_INDICATION to the output in one step
int
    msg_send_disconnect_indication (void *output,
        byte cause,
        byte diagnostic);

//  Send the COUNT to the output in one step
int
    msg_send_count (void *output);

//  Duplicate the msg message
msg_t *
    msg_dup (msg_t *self);

//  Print contents of message to stdout
void
    msg_dump (msg_t *self);

//  Get/set the message address
zframe_t *
    msg_address (msg_t *self);
const zframe_t *
    msg_const_address (const msg_t *self);
void
    msg_set_address (msg_t *self, zframe_t *address);

//  Get the msg id and printable command
int
    msg_id (msg_t *self);
int
    msg_const_id (const msg_t *self);
void
    msg_set_id (msg_t *self, int id);
char *
    msg_command (msg_t *self);
const char *
    msg_const_command (const msg_t *self);

//  Get/set the sequence field
uint32_t
    msg_sequence (msg_t *self);
uint32_t
    msg_const_sequence (const msg_t *self);
void
    msg_set_sequence (msg_t *self, uint32_t sequence);

//  Get/set the data field
zframe_t *
    msg_data (msg_t *self);
const zframe_t *
    msg_const_data (const msg_t *self);
void
    msg_set_data (msg_t *self, zframe_t *frame);

//  Get/set the service field
char *
    msg_service (msg_t *self);
const char *
    msg_const_service (const msg_t *self);
void
    msg_set_service (msg_t *self, const char *format, ...);

//  Get/set the directionality field
byte
    msg_directionality (msg_t *self);
byte
    msg_const_directionality (const msg_t *self);
void
    msg_set_directionality (msg_t *self, byte directionality);

//  Get/set the packet field
byte
    msg_packet (msg_t *self);
byte
    msg_const_packet (const msg_t *self);
void
    msg_set_packet (msg_t *self, byte packet);

//  Get/set the window field
uint16_t
    msg_window (msg_t *self);
uint16_t
    msg_const_window (const msg_t *self);
void
    msg_set_window (msg_t *self, uint16_t window);

//  Get/set the throughput field
byte
    msg_throughput (msg_t *self);
byte
    msg_const_throughput (const msg_t *self);
void
    msg_set_throughput (msg_t *self, byte throughput);

//  Get/set the cause field
byte
    msg_cause (msg_t *self);
byte
    msg_const_cause (const msg_t *self);
void
    msg_set_cause (msg_t *self, byte cause);

//  Get/set the diagnostic field
byte
    msg_diagnostic (msg_t *self);
byte
    msg_const_diagnostic (const msg_t *self);
void
    msg_set_diagnostic (msg_t *self, byte diagnostic);

//  Self test of this class
int
    msg_test (bool verbose);
//  @end

#endif
