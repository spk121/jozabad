/*  =========================================================================
    msg - transport for switched virtual call messages

    Generated codec implementation for msg
    -------------------------------------------------------------------------
    GPL3+, bitches
    =========================================================================
*/

/*
@header
    msg - transport for switched virtual call messages
@discuss
@end
*/

#define _GNU_SOURCE
#include <cstdint>
#include <czmq.h>
#include "../include/msg.h"



//  --------------------------------------------------------------------------
//  Network data encoding macros

//  Strings are encoded with 1-byte length
#define STRING_MAX  255

//  Raw blocks are encoded with a 2-byte length
#define RAW_MAX 8192

//  Put a block to the frame
#define PUT_BLOCK(host,size) { \
    memcpy (self->needle, (host), size); \
    self->needle += size; \
    }

//  Get a block from the frame
#define GET_BLOCK(host,size) { \
    if (self->needle + size > self->ceiling) \
        goto malformed; \
    memcpy ((host), self->needle, size); \
    self->needle += size; \
    }

//  Put a 1-byte number to the frame
#define PUT_NUMBER1(host) { \
    *(byte *) self->needle = (host); \
    self->needle++; \
    }

//  Put a 2-byte number to the frame
#define PUT_NUMBER2(host) { \
    self->needle [0] = (byte) (((host) >> 8)  & 255); \
    self->needle [1] = (byte) (((host))       & 255); \
    self->needle += 2; \
    }

//  Put a 4-byte number to the frame
#define PUT_NUMBER4(host) { \
    self->needle [0] = (byte) (((host) >> 24) & 255); \
    self->needle [1] = (byte) (((host) >> 16) & 255); \
    self->needle [2] = (byte) (((host) >> 8)  & 255); \
    self->needle [3] = (byte) (((host))       & 255); \
    self->needle += 4; \
    }

//  Put a 8-byte number to the frame
#define PUT_NUMBER8(host) { \
    self->needle [0] = (byte) (((host) >> 56) & 255); \
    self->needle [1] = (byte) (((host) >> 48) & 255); \
    self->needle [2] = (byte) (((host) >> 40) & 255); \
    self->needle [3] = (byte) (((host) >> 32) & 255); \
    self->needle [4] = (byte) (((host) >> 24) & 255); \
    self->needle [5] = (byte) (((host) >> 16) & 255); \
    self->needle [6] = (byte) (((host) >> 8)  & 255); \
    self->needle [7] = (byte) (((host))       & 255); \
    self->needle += 8; \
    }

//  Get a 1-byte number from the frame
#define GET_NUMBER1(host) { \
    if (self->needle + 1 > self->ceiling) \
        goto malformed; \
    (host) = *(byte *) self->needle; \
    self->needle++; \
    }

//  Get a 2-byte number from the frame
#define GET_NUMBER2(host) { \
    if (self->needle + 2 > self->ceiling) \
        goto malformed; \
    (host) = ((uint16_t) (self->needle [0]) << 8) \
           +  (uint16_t) (self->needle [1]); \
    self->needle += 2; \
    }

//  Get a 4-byte number from the frame
#define GET_NUMBER4(host) { \
    if (self->needle + 4 > self->ceiling) \
        goto malformed; \
    (host) = ((uint32_t) (self->needle [0]) << 24) \
           + ((uint32_t) (self->needle [1]) << 16) \
           + ((uint32_t) (self->needle [2]) << 8) \
           +  (uint32_t) (self->needle [3]); \
    self->needle += 4; \
    }

//  Get a 8-byte number from the frame
#define GET_NUMBER8(host) { \
    if (self->needle + 8 > self->ceiling) \
        goto malformed; \
    (host) = ((uint64_t) (self->needle [0]) << 56) \
           + ((uint64_t) (self->needle [1]) << 48) \
           + ((uint64_t) (self->needle [2]) << 40) \
           + ((uint64_t) (self->needle [3]) << 32) \
           + ((uint64_t) (self->needle [4]) << 24) \
           + ((uint64_t) (self->needle [5]) << 16) \
           + ((uint64_t) (self->needle [6]) << 8) \
           +  (uint64_t) (self->needle [7]); \
    self->needle += 8; \
    }

//  Put a string to the frame
#define PUT_STRING(host) { \
    string_size = strlen (host); \
    PUT_NUMBER1 (string_size); \
    memcpy (self->needle, (host), string_size); \
    self->needle += string_size; \
    }

//  Get a string from the frame
#define GET_STRING(host) { \
    GET_NUMBER1 (string_size); \
    if (self->needle + string_size > (self->ceiling)) \
        goto malformed; \
    (host) = (char *) malloc (string_size + 1); \
    memcpy ((host), self->needle, string_size); \
    (host) [string_size] = 0; \
    self->needle += string_size; \
    }

//  Put a raw block to the frame
#define PUT_RAW(host) { \
    string_size = (host)[0] * 256 + (host)[1] + 2; \
    (host) = (uint8_t *) malloc (string_size + 1); \
    memcpy (self->needle, (host), string_size); \
    (host) [string_size] = 0; \
    self->needle += string_size; \
    }

//  Get a raw block from the frame
#define GET_RAW(host) { \
    GET_NUMBER2 (string_size); \
    self->needle -= 2; \
    string_size += 2; \
    if (self->needle + string_size > (self->ceiling)) \
        goto malformed; \
    (host) = (uint8_t *) malloc (string_size + 1); \
    memcpy ((host), self->needle, string_size); \
    (host) [string_size] = 0; \
    self->needle += string_size; \
    }



//  --------------------------------------------------------------------------
//  Create a new msg

msg_t *
msg_new (int id)
{
    msg_t *self = (msg_t *) zmalloc (sizeof (msg_t));
    self->id = id;
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the msg

void
msg_destroy (msg_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        msg_t *self = *self_p;

        //  Free class properties
        zframe_destroy (&self->address);
        zframe_destroy (&self->data);
        free (self->called_address);
        zframe_destroy (&self->user_data);
        free (self->protocol);
        free (self->calling_address);

        //  Free object itself
        free (self);
        *self_p = NULL;
    }
}


//  --------------------------------------------------------------------------
//  Receive and parse a msg from the socket. Returns new object or
//  NULL if error. Will block if there's no message waiting.

msg_t *
msg_recv (void *input)
{
    assert (input);
    msg_t *self = msg_new (0);
    zframe_t *frame = NULL;
    size_t string_size;
    size_t list_size;
    size_t hash_size;

    //  Read valid message frame from socket; we loop over any
    //  garbage data we might receive from badly-connected peers
    while (true) {
        //  If we're reading from a ROUTER socket, get address
        if (zsockopt_type (input) == ZMQ_ROUTER) {
            zframe_destroy (&self->address);
            self->address = zframe_recv (input);
            if (!self->address)
                goto empty;         //  Interrupted
            if (!zsocket_rcvmore (input))
                goto malformed;
        }
        //  Read and parse command in frame
        frame = zframe_recv (input);
        if (!frame)
            goto empty;             //  Interrupted

        //  Get and check protocol signature
        self->needle = zframe_data (frame);
        self->ceiling = self->needle + zframe_size (frame);
        uint16_t signature;
        GET_NUMBER2 (signature);
        if (signature == (0xAAA0 | 1))
            break;                  //  Valid signature

        //  Protocol assertion, drop message
        while (zsocket_rcvmore (input)) {
            zframe_destroy (&frame);
            frame = zframe_recv (input);
        }
        zframe_destroy (&frame);
    }
    //  Get message id and parse per message type
    GET_NUMBER1 (self->id);

    switch (self->id) {
        case MSG_DATA:
            GET_NUMBER2 (self->sequence);
            //  Get next frame, leave current untouched
            if (!zsocket_rcvmore (input))
                goto malformed;
            self->data = zframe_recv (input);
            break;

        case MSG_RR:
            GET_NUMBER2 (self->sequence);
            break;

        case MSG_RNR:
            GET_NUMBER2 (self->sequence);
            break;

        case MSG_CALL_REQUEST:
            free (self->called_address);
            GET_STRING (self->called_address);
            GET_NUMBER1 (self->packet);
            GET_NUMBER2 (self->window);
            GET_NUMBER1 (self->throughput);
            //  Get next frame, leave current untouched
            if (!zsocket_rcvmore (input))
                goto malformed;
            self->user_data = zframe_recv (input);
            break;

        case MSG_CALL_ACCEPTED:
            GET_NUMBER1 (self->packet);
            GET_NUMBER2 (self->window);
            GET_NUMBER1 (self->throughput);
            //  Get next frame, leave current untouched
            if (!zsocket_rcvmore (input))
                goto malformed;
            self->user_data = zframe_recv (input);
            break;

        case MSG_CLEAR_REQUEST:
            GET_NUMBER1 (self->cause);
            GET_NUMBER1 (self->diagnostic);
            break;

        case MSG_CLEAR_CONFIRMATION:
            break;

        case MSG_RESET_REQUEST:
            GET_NUMBER1 (self->cause);
            GET_NUMBER1 (self->diagnostic);
            break;

        case MSG_RESET_CONFIRMATION:
            GET_NUMBER1 (self->cause);
            GET_NUMBER1 (self->diagnostic);
            break;

        case MSG_CONNECT:
            free (self->protocol);
            GET_STRING (self->protocol);
            if (strneq (self->protocol, "~SVC"))
                goto malformed;
            GET_NUMBER1 (self->version);
            if (self->version != MSG_VERSION)
                goto malformed;
            free (self->calling_address);
            GET_STRING (self->calling_address);
            GET_NUMBER1 (self->directionality);
            GET_NUMBER1 (self->throughput);
            break;

        case MSG_CONNECT_INDICATION:
            GET_NUMBER1 (self->directionality);
            GET_NUMBER1 (self->throughput);
            break;

        case MSG_DISCONNECT:
            break;

        case MSG_DISCONNECT_INDICATION:
            GET_NUMBER1 (self->cause);
            GET_NUMBER1 (self->diagnostic);
            break;

        case MSG_COUNT:
            break;

        default:
            goto malformed;
    }
    //  Successful return
    zframe_destroy (&frame);
    return self;

    //  Error returns
    malformed:
        printf ("E: malformed message '%d'\n", self->id);
    empty:
        zframe_destroy (&frame);
        msg_destroy (&self);
        return (NULL);
}


//  --------------------------------------------------------------------------
//  Send the msg to the socket, and destroy it
//  Returns 0 if OK, else -1

int
msg_send (msg_t **self_p, void *output)
{
    assert (output);
    assert (self_p);
    assert (*self_p);

    //  Calculate size of serialized data
    msg_t *self = *self_p;
    size_t frame_size = 2 + 1;          //  Signature and message ID
    switch (self->id) {
        case MSG_DATA:
            //  sequence is a 2-byte integer
            frame_size += 2;
            break;

        case MSG_RR:
            //  sequence is a 2-byte integer
            frame_size += 2;
            break;

        case MSG_RNR:
            //  sequence is a 2-byte integer
            frame_size += 2;
            break;

        case MSG_CALL_REQUEST:
            //  called_address is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->called_address)
                frame_size += strlen (self->called_address);
            //  packet is a 1-byte integer
            frame_size += 1;
            //  window is a 2-byte integer
            frame_size += 2;
            //  throughput is a 1-byte integer
            frame_size += 1;
            break;

        case MSG_CALL_ACCEPTED:
            //  packet is a 1-byte integer
            frame_size += 1;
            //  window is a 2-byte integer
            frame_size += 2;
            //  throughput is a 1-byte integer
            frame_size += 1;
            break;

        case MSG_CLEAR_REQUEST:
            //  cause is a 1-byte integer
            frame_size += 1;
            //  diagnostic is a 1-byte integer
            frame_size += 1;
            break;

        case MSG_CLEAR_CONFIRMATION:
            break;

        case MSG_RESET_REQUEST:
            //  cause is a 1-byte integer
            frame_size += 1;
            //  diagnostic is a 1-byte integer
            frame_size += 1;
            break;

        case MSG_RESET_CONFIRMATION:
            //  cause is a 1-byte integer
            frame_size += 1;
            //  diagnostic is a 1-byte integer
            frame_size += 1;
            break;

        case MSG_CONNECT:
            //  protocol is a string with 1-byte length
            frame_size += 1 + strlen ("~SVC");
            //  version is a 1-byte integer
            frame_size += 1;
            //  calling_address is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->calling_address)
                frame_size += strlen (self->calling_address);
            //  directionality is a 1-byte integer
            frame_size += 1;
            //  throughput is a 1-byte integer
            frame_size += 1;
            break;

        case MSG_CONNECT_INDICATION:
            //  directionality is a 1-byte integer
            frame_size += 1;
            //  throughput is a 1-byte integer
            frame_size += 1;
            break;

        case MSG_DISCONNECT:
            break;

        case MSG_DISCONNECT_INDICATION:
            //  cause is a 1-byte integer
            frame_size += 1;
            //  diagnostic is a 1-byte integer
            frame_size += 1;
            break;

        case MSG_COUNT:
            break;

        default:
            printf ("E: bad message type '%d', not sent\n", self->id);
            //  No recovery, this is a fatal application error
            assert (false);
    }
    //  Now serialize message into the frame
    zframe_t *frame = zframe_new (NULL, frame_size);
    self->needle = zframe_data (frame);
    size_t string_size;
    int frame_flags = 0;
    PUT_NUMBER2 (0xAAA0 | 1);
    PUT_NUMBER1 (self->id);

    switch (self->id) {
        case MSG_DATA:
            PUT_NUMBER2 (self->sequence);
            frame_flags = ZFRAME_MORE;
            break;

        case MSG_RR:
            PUT_NUMBER2 (self->sequence);
            break;

        case MSG_RNR:
            PUT_NUMBER2 (self->sequence);
            break;

        case MSG_CALL_REQUEST:
            if (self->called_address) {
                PUT_STRING (self->called_address);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            PUT_NUMBER1 (self->packet);
            PUT_NUMBER2 (self->window);
            PUT_NUMBER1 (self->throughput);
            frame_flags = ZFRAME_MORE;
            break;

        case MSG_CALL_ACCEPTED:
            PUT_NUMBER1 (self->packet);
            PUT_NUMBER2 (self->window);
            PUT_NUMBER1 (self->throughput);
            frame_flags = ZFRAME_MORE;
            break;

        case MSG_CLEAR_REQUEST:
            PUT_NUMBER1 (self->cause);
            PUT_NUMBER1 (self->diagnostic);
            break;

        case MSG_CLEAR_CONFIRMATION:
            break;

        case MSG_RESET_REQUEST:
            PUT_NUMBER1 (self->cause);
            PUT_NUMBER1 (self->diagnostic);
            break;

        case MSG_RESET_CONFIRMATION:
            PUT_NUMBER1 (self->cause);
            PUT_NUMBER1 (self->diagnostic);
            break;

        case MSG_CONNECT:
            PUT_STRING ("~SVC");
            PUT_NUMBER1 (MSG_VERSION);
            if (self->calling_address) {
                PUT_STRING (self->calling_address);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            PUT_NUMBER1 (self->directionality);
            PUT_NUMBER1 (self->throughput);
            break;

        case MSG_CONNECT_INDICATION:
            PUT_NUMBER1 (self->directionality);
            PUT_NUMBER1 (self->throughput);
            break;

        case MSG_DISCONNECT:
            break;

        case MSG_DISCONNECT_INDICATION:
            PUT_NUMBER1 (self->cause);
            PUT_NUMBER1 (self->diagnostic);
            break;

        case MSG_COUNT:
            break;

    }
    //  If we're sending to a ROUTER, we send the address first
    if (zsockopt_type (output) == ZMQ_ROUTER) {
        assert (self->address);
        if (zframe_send (&self->address, output, ZFRAME_MORE)) {
            zframe_destroy (&frame);
            msg_destroy (self_p);
            return -1;
        }
    }
    //  Now send the data frame
    if (zframe_send (&frame, output, frame_flags)) {
        zframe_destroy (&frame);
        msg_destroy (self_p);
        return -1;
    }

    //  Now send any frame fields, in order
    switch (self->id) {
        case MSG_DATA:
            //  If data isn't set, send an empty frame
            if (!self->data)
                self->data = zframe_new (NULL, 0);
            if (zframe_send (&self->data, output, 0)) {
                zframe_destroy (&frame);
                msg_destroy (self_p);
                return -1;
            }
            break;
        case MSG_CALL_REQUEST:
            //  If user_data isn't set, send an empty frame
            if (!self->user_data)
                self->user_data = zframe_new (NULL, 0);
            if (zframe_send (&self->user_data, output, 0)) {
                zframe_destroy (&frame);
                msg_destroy (self_p);
                return -1;
            }
            break;
        case MSG_CALL_ACCEPTED:
            //  If user_data isn't set, send an empty frame
            if (!self->user_data)
                self->user_data = zframe_new (NULL, 0);
            if (zframe_send (&self->user_data, output, 0)) {
                zframe_destroy (&frame);
                msg_destroy (self_p);
                return -1;
            }
            break;
    }
    //  Destroy msg object
    msg_destroy (self_p);
    return 0;
}


//  --------------------------------------------------------------------------
//  Send the DATA to the socket in one step

int
msg_send_data (
    void *output,
    uint16_t sequence,
    zframe_t *data)
{
    msg_t *self = msg_new (MSG_DATA);
    msg_set_sequence (self, sequence);
    msg_set_data (self, zframe_dup (data));
    return msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the RR to the socket in one step

int
msg_send_rr (
    void *output,
    uint16_t sequence)
{
    msg_t *self = msg_new (MSG_RR);
    msg_set_sequence (self, sequence);
    return msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the RNR to the socket in one step

int
msg_send_rnr (
    void *output,
    uint16_t sequence)
{
    msg_t *self = msg_new (MSG_RNR);
    msg_set_sequence (self, sequence);
    return msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the CALL_REQUEST to the socket in one step

int
msg_send_call_request (
    void *output,
    char *called_address,
    byte packet,
    uint16_t window,
    byte throughput,
    zframe_t *user_data)
{
    msg_t *self = msg_new (MSG_CALL_REQUEST);
    msg_set_called_address (self, called_address);
    msg_set_packet (self, packet);
    msg_set_window (self, window);
    msg_set_throughput (self, throughput);
    msg_set_user_data (self, zframe_dup (user_data));
    return msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the CALL_ACCEPTED to the socket in one step

int
msg_send_call_accepted (
    void *output,
    byte packet,
    uint16_t window,
    byte throughput,
    zframe_t *user_data)
{
    msg_t *self = msg_new (MSG_CALL_ACCEPTED);
    msg_set_packet (self, packet);
    msg_set_window (self, window);
    msg_set_throughput (self, throughput);
    msg_set_user_data (self, zframe_dup (user_data));
    return msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the CLEAR_REQUEST to the socket in one step

int
msg_send_clear_request (
    void *output,
    byte cause,
    byte diagnostic)
{
    msg_t *self = msg_new (MSG_CLEAR_REQUEST);
    msg_set_cause (self, cause);
    msg_set_diagnostic (self, diagnostic);
    return msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the CLEAR_CONFIRMATION to the socket in one step

int
msg_send_clear_confirmation (
    void *output)
{
    msg_t *self = msg_new (MSG_CLEAR_CONFIRMATION);
    return msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the RESET_REQUEST to the socket in one step

int
msg_send_reset_request (
    void *output,
    byte cause,
    byte diagnostic)
{
    msg_t *self = msg_new (MSG_RESET_REQUEST);
    msg_set_cause (self, cause);
    msg_set_diagnostic (self, diagnostic);
    return msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the RESET_CONFIRMATION to the socket in one step

int
msg_send_reset_confirmation (
    void *output,
    byte cause,
    byte diagnostic)
{
    msg_t *self = msg_new (MSG_RESET_CONFIRMATION);
    msg_set_cause (self, cause);
    msg_set_diagnostic (self, diagnostic);
    return msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the CONNECT to the socket in one step

int
msg_send_connect (
    void *output,
    char *calling_address,
    byte directionality,
    byte throughput)
{
    msg_t *self = msg_new (MSG_CONNECT);
    msg_set_calling_address (self, calling_address);
    msg_set_directionality (self, directionality);
    msg_set_throughput (self, throughput);
    return msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the CONNECT_INDICATION to the socket in one step

int
msg_send_connect_indication (
    void *output,
    byte directionality,
    byte throughput)
{
    msg_t *self = msg_new (MSG_CONNECT_INDICATION);
    msg_set_directionality (self, directionality);
    msg_set_throughput (self, throughput);
    return msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the DISCONNECT to the socket in one step

int
msg_send_disconnect (
    void *output)
{
    msg_t *self = msg_new (MSG_DISCONNECT);
    return msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the DISCONNECT_INDICATION to the socket in one step

int
msg_send_disconnect_indication (
    void *output,
    byte cause,
    byte diagnostic)
{
    msg_t *self = msg_new (MSG_DISCONNECT_INDICATION);
    msg_set_cause (self, cause);
    msg_set_diagnostic (self, diagnostic);
    return msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the COUNT to the socket in one step

int
msg_send_count (
    void *output)
{
    msg_t *self = msg_new (MSG_COUNT);
    return msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Duplicate the msg message

msg_t *
msg_dup (msg_t *self)
{
    if (!self)
        return NULL;

    msg_t *copy = msg_new (self->id);
    if (self->address)
        copy->address = zframe_dup (self->address);
    switch (self->id) {
        case MSG_DATA:
            copy->sequence = self->sequence;
            copy->data = zframe_dup (self->data);
            break;

        case MSG_RR:
            copy->sequence = self->sequence;
            break;

        case MSG_RNR:
            copy->sequence = self->sequence;
            break;

        case MSG_CALL_REQUEST:
            copy->called_address = strdup (self->called_address);
            copy->packet = self->packet;
            copy->window = self->window;
            copy->throughput = self->throughput;
            copy->user_data = zframe_dup (self->user_data);
            break;

        case MSG_CALL_ACCEPTED:
            copy->packet = self->packet;
            copy->window = self->window;
            copy->throughput = self->throughput;
            copy->user_data = zframe_dup (self->user_data);
            break;

        case MSG_CLEAR_REQUEST:
            copy->cause = self->cause;
            copy->diagnostic = self->diagnostic;
            break;

        case MSG_CLEAR_CONFIRMATION:
            break;

        case MSG_RESET_REQUEST:
            copy->cause = self->cause;
            copy->diagnostic = self->diagnostic;
            break;

        case MSG_RESET_CONFIRMATION:
            copy->cause = self->cause;
            copy->diagnostic = self->diagnostic;
            break;

        case MSG_CONNECT:
            copy->protocol = strdup (self->protocol);
            copy->version = self->version;
            copy->calling_address = strdup (self->calling_address);
            copy->directionality = self->directionality;
            copy->throughput = self->throughput;
            break;

        case MSG_CONNECT_INDICATION:
            copy->directionality = self->directionality;
            copy->throughput = self->throughput;
            break;

        case MSG_DISCONNECT:
            break;

        case MSG_DISCONNECT_INDICATION:
            copy->cause = self->cause;
            copy->diagnostic = self->diagnostic;
            break;

        case MSG_COUNT:
            break;

    }
    return copy;
}



//  --------------------------------------------------------------------------
//  Print contents of message to stdout

void
msg_dump (msg_t *self)
{
    assert (self);
    switch (self->id) {
        case MSG_DATA:
            puts ("DATA:");
            printf ("    sequence=%ld\n", (long) self->sequence);
            printf ("    data={\n");
            if (self->data) {
                size_t size = zframe_size (self->data);
                byte *data = zframe_data (self->data);
                printf ("        size=%td\n", zframe_size (self->data));
                if (size > 32)
                    size = 32;
                int data_index;
                for (data_index = 0; data_index < size; data_index++) {
                    if (data_index && (data_index % 4 == 0))
                        printf ("-");
                    printf ("%02X", data [data_index]);
                }
            }
            printf ("    }\n");
            break;

        case MSG_RR:
            puts ("RR:");
            printf ("    sequence=%ld\n", (long) self->sequence);
            break;

        case MSG_RNR:
            puts ("RNR:");
            printf ("    sequence=%ld\n", (long) self->sequence);
            break;

        case MSG_CALL_REQUEST:
            puts ("CALL_REQUEST:");
            if (self->called_address)
                printf ("    called_address='%s'\n", self->called_address);
            else
                printf ("    called_address=\n");
            printf ("    packet=%ld\n", (long) self->packet);
            printf ("    window=%ld\n", (long) self->window);
            printf ("    throughput=%ld\n", (long) self->throughput);
            printf ("    user_data={\n");
            if (self->user_data) {
                size_t size = zframe_size (self->user_data);
                byte *data = zframe_data (self->user_data);
                printf ("        size=%td\n", zframe_size (self->user_data));
                if (size > 32)
                    size = 32;
                int user_data_index;
                for (user_data_index = 0; user_data_index < size; user_data_index++) {
                    if (user_data_index && (user_data_index % 4 == 0))
                        printf ("-");
                    printf ("%02X", data [user_data_index]);
                }
            }
            printf ("    }\n");
            break;

        case MSG_CALL_ACCEPTED:
            puts ("CALL_ACCEPTED:");
            printf ("    packet=%ld\n", (long) self->packet);
            printf ("    window=%ld\n", (long) self->window);
            printf ("    throughput=%ld\n", (long) self->throughput);
            printf ("    user_data={\n");
            if (self->user_data) {
                size_t size = zframe_size (self->user_data);
                byte *data = zframe_data (self->user_data);
                printf ("        size=%td\n", zframe_size (self->user_data));
                if (size > 32)
                    size = 32;
                int user_data_index;
                for (user_data_index = 0; user_data_index < size; user_data_index++) {
                    if (user_data_index && (user_data_index % 4 == 0))
                        printf ("-");
                    printf ("%02X", data [user_data_index]);
                }
            }
            printf ("    }\n");
            break;

        case MSG_CLEAR_REQUEST:
            puts ("CLEAR_REQUEST:");
            printf ("    cause=%ld\n", (long) self->cause);
            printf ("    diagnostic=%ld\n", (long) self->diagnostic);
            break;

        case MSG_CLEAR_CONFIRMATION:
            puts ("CLEAR_CONFIRMATION:");
            break;

        case MSG_RESET_REQUEST:
            puts ("RESET_REQUEST:");
            printf ("    cause=%ld\n", (long) self->cause);
            printf ("    diagnostic=%ld\n", (long) self->diagnostic);
            break;

        case MSG_RESET_CONFIRMATION:
            puts ("RESET_CONFIRMATION:");
            printf ("    cause=%ld\n", (long) self->cause);
            printf ("    diagnostic=%ld\n", (long) self->diagnostic);
            break;

        case MSG_CONNECT:
            puts ("CONNECT:");
            printf ("    protocol=~svc\n");
            printf ("    version=msg_version\n");
            if (self->calling_address)
                printf ("    calling_address='%s'\n", self->calling_address);
            else
                printf ("    calling_address=\n");
            printf ("    directionality=%ld\n", (long) self->directionality);
            printf ("    throughput=%ld\n", (long) self->throughput);
            break;

        case MSG_CONNECT_INDICATION:
            puts ("CONNECT_INDICATION:");
            printf ("    directionality=%ld\n", (long) self->directionality);
            printf ("    throughput=%ld\n", (long) self->throughput);
            break;

        case MSG_DISCONNECT:
            puts ("DISCONNECT:");
            break;

        case MSG_DISCONNECT_INDICATION:
            puts ("DISCONNECT_INDICATION:");
            printf ("    cause=%ld\n", (long) self->cause);
            printf ("    diagnostic=%ld\n", (long) self->diagnostic);
            break;

        case MSG_COUNT:
            puts ("COUNT:");
            break;

    }
}


//  --------------------------------------------------------------------------
//  Get/set the message address

zframe_t *
msg_address (msg_t *self)
{
    assert (self);
    return self->address;
}

const zframe_t *
msg_const_address (const msg_t *self)
{
    assert (self);
    return self->address;
}

void
msg_set_address (msg_t *self, zframe_t *address)
{
    if (self->address)
        zframe_destroy (&self->address);
    self->address = zframe_dup (address);
}


//  --------------------------------------------------------------------------
//  Get/set the msg id

int
msg_id (msg_t *self)
{
    assert (self);
    return self->id;
}

int
msg_const_id (const msg_t *self)
{
    assert (self);
    return self->id;
}

void
msg_set_id (msg_t *self, int id)
{
    self->id = id;
}

//  --------------------------------------------------------------------------
//  Return a printable command string

const char *
msg_const_command (const msg_t *self)
{
    assert (self);
    switch (self->id) {
        case MSG_DATA:
            return ("DATA");
            break;
        case MSG_RR:
            return ("RR");
            break;
        case MSG_RNR:
            return ("RNR");
            break;
        case MSG_CALL_REQUEST:
            return ("CALL_REQUEST");
            break;
        case MSG_CALL_ACCEPTED:
            return ("CALL_ACCEPTED");
            break;
        case MSG_CLEAR_REQUEST:
            return ("CLEAR_REQUEST");
            break;
        case MSG_CLEAR_CONFIRMATION:
            return ("CLEAR_CONFIRMATION");
            break;
        case MSG_RESET_REQUEST:
            return ("RESET_REQUEST");
            break;
        case MSG_RESET_CONFIRMATION:
            return ("RESET_CONFIRMATION");
            break;
        case MSG_CONNECT:
            return ("CONNECT");
            break;
        case MSG_CONNECT_INDICATION:
            return ("CONNECT_INDICATION");
            break;
        case MSG_DISCONNECT:
            return ("DISCONNECT");
            break;
        case MSG_DISCONNECT_INDICATION:
            return ("DISCONNECT_INDICATION");
            break;
        case MSG_COUNT:
            return ("COUNT");
            break;
    }
    return "?";
}

//  --------------------------------------------------------------------------
//  Get/set the sequence field

uint16_t
msg_sequence (msg_t *self)
{
    assert (self);
    return self->sequence;
}

uint16_t
msg_const_sequence (const msg_t *self)
{
    assert (self);
    return self->sequence;
}

void
msg_set_sequence (msg_t *self, uint16_t sequence)
{
    assert (self);
    self->sequence = sequence;
}


//  --------------------------------------------------------------------------
//  Get/set the data field

zframe_t *
msg_data (msg_t *self)
{
    assert (self);
    return self->data;
}

const zframe_t *
msg_const_data (const msg_t *self)
{
    assert (self);
    return self->data;
}

//  Takes ownership of supplied frame
void
msg_set_data (msg_t *self, zframe_t *frame)
{
    assert (self);
    if (self->data)
        zframe_destroy (&self->data);
    self->data = frame;
}

//  --------------------------------------------------------------------------
//  Get/set the called_address field

char *
msg_called_address (msg_t *self)
{
    assert (self);
    return self->called_address;
}

const char *
msg_const_called_address (const msg_t *self)
{
    assert (self);
    return self->called_address;
}

void
msg_set_called_address (msg_t *self, const char *format, ...)
{
    //  Format into newly allocated string
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->called_address);
    self->called_address = (char *) malloc (STRING_MAX + 1);
    assert (self->called_address);
    vsnprintf (self->called_address, STRING_MAX, format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the packet field

byte
msg_packet (msg_t *self)
{
    assert (self);
    return self->packet;
}

byte
msg_const_packet (const msg_t *self)
{
    assert (self);
    return self->packet;
}

void
msg_set_packet (msg_t *self, byte packet)
{
    assert (self);
    self->packet = packet;
}


//  --------------------------------------------------------------------------
//  Get/set the window field

uint16_t
msg_window (msg_t *self)
{
    assert (self);
    return self->window;
}

uint16_t
msg_const_window (const msg_t *self)
{
    assert (self);
    return self->window;
}

void
msg_set_window (msg_t *self, uint16_t window)
{
    assert (self);
    self->window = window;
}


//  --------------------------------------------------------------------------
//  Get/set the throughput field

byte
msg_throughput (msg_t *self)
{
    assert (self);
    return self->throughput;
}

byte
msg_const_throughput (const msg_t *self)
{
    assert (self);
    return self->throughput;
}

void
msg_set_throughput (msg_t *self, byte throughput)
{
    assert (self);
    self->throughput = throughput;
}


//  --------------------------------------------------------------------------
//  Get/set the user_data field

zframe_t *
msg_user_data (msg_t *self)
{
    assert (self);
    return self->user_data;
}

const zframe_t *
msg_const_user_data (const msg_t *self)
{
    assert (self);
    return self->user_data;
}

//  Takes ownership of supplied frame
void
msg_set_user_data (msg_t *self, zframe_t *frame)
{
    assert (self);
    if (self->user_data)
        zframe_destroy (&self->user_data);
    self->user_data = frame;
}

//  --------------------------------------------------------------------------
//  Get/set the cause field

byte
msg_cause (msg_t *self)
{
    assert (self);
    return self->cause;
}

byte
msg_const_cause (const msg_t *self)
{
    assert (self);
    return self->cause;
}

void
msg_set_cause (msg_t *self, byte cause)
{
    assert (self);
    self->cause = cause;
}


//  --------------------------------------------------------------------------
//  Get/set the diagnostic field

byte
msg_diagnostic (msg_t *self)
{
    assert (self);
    return self->diagnostic;
}

byte
msg_const_diagnostic (const msg_t *self)
{
    assert (self);
    return self->diagnostic;
}

void
msg_set_diagnostic (msg_t *self, byte diagnostic)
{
    assert (self);
    self->diagnostic = diagnostic;
}


//  --------------------------------------------------------------------------
//  Get/set the calling_address field

char *
msg_calling_address (msg_t *self)
{
    assert (self);
    return self->calling_address;
}

const char *
msg_const_calling_address (const msg_t *self)
{
    assert (self);
    return self->calling_address;
}

void
msg_set_calling_address (msg_t *self, const char *format, ...)
{
    //  Format into newly allocated string
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->calling_address);
    self->calling_address = (char *) malloc (STRING_MAX + 1);
    assert (self->calling_address);
    vsnprintf (self->calling_address, STRING_MAX, format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the directionality field

byte
msg_directionality (msg_t *self)
{
    assert (self);
    return self->directionality;
}

byte
msg_const_directionality (const msg_t *self)
{
    assert (self);
    return self->directionality;
}

void
msg_set_directionality (msg_t *self, byte directionality)
{
    assert (self);
    self->directionality = directionality;
}



//  --------------------------------------------------------------------------
//  Selftest

int
msg_test (bool verbose)
{
    printf (" * msg: ");

    //  @selftest
    //  Simple create/destroy test
    msg_t *self = msg_new (0);
    assert (self);
    msg_destroy (&self);

    //  Create pair of sockets we can send through
    zctx_t *ctx = zctx_new ();
    assert (ctx);

    void *output = zsocket_new (ctx, ZMQ_DEALER);
    assert (output);
    zsocket_bind (output, "inproc://selftest");
    void *input = zsocket_new (ctx, ZMQ_ROUTER);
    assert (input);
    zsocket_connect (input, "inproc://selftest");

    //  Encode/send/decode and verify each message type

    self = msg_new (MSG_DATA);
    msg_set_sequence (self, 123);
    msg_set_data (self, zframe_new ("Captcha Diem", 12));
    msg_send (&self, output);

    self = msg_recv (input);
    assert (self);
    assert (msg_sequence (self) == 123);
    assert (zframe_streq (msg_data (self), "Captcha Diem"));
    msg_destroy (&self);

    self = msg_new (MSG_RR);
    msg_set_sequence (self, 123);
    msg_send (&self, output);

    self = msg_recv (input);
    assert (self);
    assert (msg_sequence (self) == 123);
    msg_destroy (&self);

    self = msg_new (MSG_RNR);
    msg_set_sequence (self, 123);
    msg_send (&self, output);

    self = msg_recv (input);
    assert (self);
    assert (msg_sequence (self) == 123);
    msg_destroy (&self);

    self = msg_new (MSG_CALL_REQUEST);
    msg_set_called_address (self, "Life is short but Now lasts for ever");
    msg_set_packet (self, 123);
    msg_set_window (self, 123);
    msg_set_throughput (self, 123);
    msg_set_user_data (self, zframe_new ("Captcha Diem", 12));
    msg_send (&self, output);

    self = msg_recv (input);
    assert (self);
    assert (streq (msg_called_address (self), "Life is short but Now lasts for ever"));
    assert (msg_packet (self) == 123);
    assert (msg_window (self) == 123);
    assert (msg_throughput (self) == 123);
    assert (zframe_streq (msg_user_data (self), "Captcha Diem"));
    msg_destroy (&self);

    self = msg_new (MSG_CALL_ACCEPTED);
    msg_set_packet (self, 123);
    msg_set_window (self, 123);
    msg_set_throughput (self, 123);
    msg_set_user_data (self, zframe_new ("Captcha Diem", 12));
    msg_send (&self, output);

    self = msg_recv (input);
    assert (self);
    assert (msg_packet (self) == 123);
    assert (msg_window (self) == 123);
    assert (msg_throughput (self) == 123);
    assert (zframe_streq (msg_user_data (self), "Captcha Diem"));
    msg_destroy (&self);

    self = msg_new (MSG_CLEAR_REQUEST);
    msg_set_cause (self, 123);
    msg_set_diagnostic (self, 123);
    msg_send (&self, output);

    self = msg_recv (input);
    assert (self);
    assert (msg_cause (self) == 123);
    assert (msg_diagnostic (self) == 123);
    msg_destroy (&self);

    self = msg_new (MSG_CLEAR_CONFIRMATION);
    msg_send (&self, output);

    self = msg_recv (input);
    assert (self);
    msg_destroy (&self);

    self = msg_new (MSG_RESET_REQUEST);
    msg_set_cause (self, 123);
    msg_set_diagnostic (self, 123);
    msg_send (&self, output);

    self = msg_recv (input);
    assert (self);
    assert (msg_cause (self) == 123);
    assert (msg_diagnostic (self) == 123);
    msg_destroy (&self);

    self = msg_new (MSG_RESET_CONFIRMATION);
    msg_set_cause (self, 123);
    msg_set_diagnostic (self, 123);
    msg_send (&self, output);

    self = msg_recv (input);
    assert (self);
    assert (msg_cause (self) == 123);
    assert (msg_diagnostic (self) == 123);
    msg_destroy (&self);

    self = msg_new (MSG_CONNECT);
    msg_set_calling_address (self, "Life is short but Now lasts for ever");
    msg_set_directionality (self, 123);
    msg_set_throughput (self, 123);
    msg_send (&self, output);

    self = msg_recv (input);
    assert (self);
    assert (streq (msg_calling_address (self), "Life is short but Now lasts for ever"));
    assert (msg_directionality (self) == 123);
    assert (msg_throughput (self) == 123);
    msg_destroy (&self);

    self = msg_new (MSG_CONNECT_INDICATION);
    msg_set_directionality (self, 123);
    msg_set_throughput (self, 123);
    msg_send (&self, output);

    self = msg_recv (input);
    assert (self);
    assert (msg_directionality (self) == 123);
    assert (msg_throughput (self) == 123);
    msg_destroy (&self);

    self = msg_new (MSG_DISCONNECT);
    msg_send (&self, output);

    self = msg_recv (input);
    assert (self);
    msg_destroy (&self);

    self = msg_new (MSG_DISCONNECT_INDICATION);
    msg_set_cause (self, 123);
    msg_set_diagnostic (self, 123);
    msg_send (&self, output);

    self = msg_recv (input);
    assert (self);
    assert (msg_cause (self) == 123);
    assert (msg_diagnostic (self) == 123);
    msg_destroy (&self);

    self = msg_new (MSG_COUNT);
    msg_send (&self, output);

    self = msg_recv (input);
    assert (self);
    msg_destroy (&self);

    zctx_destroy (&ctx);
    //  @end

    printf ("OK\n");
    return 0;
}
