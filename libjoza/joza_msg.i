/*  =========================================================================
    joza_msg - transport for switched virtual call messages

    Generated codec header for joza_msg
    -------------------------------------------------------------------------
    GPL3+, bitches
    =========================================================================
*/

%module joza
%{
#define _GNU_SOURCE
#define _LARGEFILE64_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <zmq.h>
#include <czmq.h>
#include "joza_msg.h"
#include "joza_lib.h"
%}

#define _LARGEFILE64_SOURCE
#include <czmq.h>
#include <stdio.h>
#include "joza_lib.h"
#include "joza_msg.h"

zctx_t *zctx_new (void);
void zctx_destroy (zctx_t **self_p);
void zclock_sleep (int msecs);
int64_t zclock_time (void);
void zclock_log (const char *format, ...);
int zmq_connect (void *socket, const char *endpoint);
void *zsocket_new (zctx_t *self, int type);
void  zsocket_destroy (zctx_t *self, void *socket);
int  zsocket_bind (void *socket, const char *format, ...);
int zsocket_connect (void *socket, const char *format, ...);
#define ZMQ_DEALER 5
#define WINDOW_MIN 1U
#define WINDOW_MAX 32767U
#define WINDOW_DEFAULT 128U
    
typedef enum _cause_t {
    c_unspecified,
    c_worker_originated,
    c_number_busy,
    c_out_of_order,
    c_remote_procedure_error,
    c_reverse_charging_acceptance_not_subscribed,
    c_incompatible_destination,
    c_fast_select_acceptance_not_subscribed,
    c_ship_absent,
    c_invalid_facility_request,
    c_access_barred,
    c_local_procedure_error,
    c_network_congestion,
    c_not_obtainable,
    c_ROA_out_of_order,
    
    c_last = c_ROA_out_of_order
} cause_t;
    
typedef enum _diagnostic_t {
    d_unspecified,
    d_invalid_ps,
    d_invalid_pr,
        
    d_packet_type_invalid,
    d_packet_type_invalid_for_s1,
    d_packet_type_invalid_for_s2,
    d_packet_type_invalid_for_s3,
    d_packet_type_invalid_for_s4,
    d_packet_type_invalid_for_s5,
    d_packet_type_invalid_for_s6,
    d_packet_type_invalid_for_s7,
    d_packet_type_invalid_for_s8,
    d_packet_type_invalid_for_s9,

    d_packet_not_allowed,
    d_unidentifiable_packet,
    d_call_on_one_way_logical_channel,
    d_invalid_packet_type_on_permanent_virtual_circuit,
    d_packet_on_unassigned_logical_channel,
    d_reject_not_subscribed_to,
    d_packet_too_short,
    d_packet_too_long,
    d_invalid_general_format_specifier,
    d_restart_packet_with_non_zero_logical_channel,
    d_packet_type_not_compatible_with_facility,
    d_unauthorized_interrupt_confirmation,
    d_unauthorized_interrupt,
    d_unaurhorized_reject,
    d_TOA_NPI_address_subscription_facility_not_subscribed_to,

    d_time_expired,
    d_time_expired_for_incoming_call,
    d_time_expired_for_clear_indication,
    d_time_expired_for_reset_indication,
    d_time_expired_for_restart_indication,
    d_time_expired_for_call_deflection,
        
    d_call_set_up_or_clearing_problem,
    d_facility_code_not_allowed,
    d_facility_parameter_not_allowed,
    d_invalid_called_address,
    d_invalid_calling_address,
    d_invalid_facility_length,
    d_incoming_call_barred,
    d_no_logical_channel_available,
    d_call_collision,
    d_duplicate_facility_requested,
    d_non_zero_address_length,
    d_non_zero_facility_length,
   d_facility_not_provided_when_expected,
    d_maximum_number_of_call_redirections_exceeded,

    d_miscellaneous,
    d_improper_cause_code_from_worker,
    d_not_aligned_octet,
    d_inconsistent_q_bit_setting,
    d_NUI_problem,
    d_ICRD_problem,

    d_not_assigned,

    d_international_problem,
    d_remote_network_problem,
    d_international_protocol_problem,
    d_international_link_out_of_order,
    d_international_link_busy,
    d_transit_network_facility_problem,
    d_remote_network_facility_problem,
    d_international_routing_problem,
    d_temporary_routing_problem,
    d_unknown_called_DNIC,
    d_maintenance_action,

    d_last = d_maintenance_action
} diagnostic_t;
    
// Note that bit 0 is input barred and bit 1 is output barred
typedef enum _direction_t {
    direction_bidirectional = 0,
    direction_input_barred = 1,
    direction_output_barred = 2,
    direction_io_barred = 3,

    direction_default = direction_bidirectional,
    direction_last = direction_io_barred
} direction_t;
    
typedef enum _packet_t {
    p_unspecified = 0,
    p_reserved = 1,
    p_reserved2 = 2,
    p_reserved3 = 3,
    p_16_bytes = 4,
    p_32_bytes = 5,
    p_64_bytes = 6,
    p_128_bytes = 7,
    p_256_bytes = 8,
    p_512_bytes = 9,
    p_1_Kbyte = 10,
    p_2_Kbytes = 11,
    p_4_Kbytes = 12,
        
    p_first = p_16_bytes,
    p_default = p_128_bytes,
    p_last = p_4_Kbytes
} packet_t;

typedef enum _throughput_t {
    t_unspecified,
    t_reserved_1,
    t_reserved_2,
    t_75bps,
    t_150bps,
    t_300bps,
    t_600bps,
    t_1200bps,
    t_2400bps,
    t_4800bps,
    t_9600bps,
    t_19200bps,
    t_48kpbs,
    t_64kbps,
    t_128kbps,
    t_192kbps,
    t_256kbps,
    t_320kbps,
    t_384kbps,
    t_448kbps,
    t_512kbps,
    t_578kbps,
    t_640kbps,
    t_704kbps,
    t_768kbps,
    t_832kbps,
    t_896kbps,
    t_960kbps,
    t_1024kbps,
    t_1088kbps,
    t_1152kbps,
    t_1216kbps,
    t_1280kbps,
    t_1344kbps,
    t_1408kbps,
    t_1472kbps,
    t_1536kbps,
    t_1600kbps,
    t_1664kbps,
    t_1728kbps,
    t_1792kbps,
    t_1856kbps,
    t_1920kbps,
    t_1984kbps,
    t_2048kbps,
        
    t_first = t_75bps,
    t_default = t_64kbps,
    t_last = t_2048kbps
} throughput_t;
    
const char *cause_name(cause_t c);
const char *diagnostic_name(diagnostic_t c);
const char *direction_name(direction_t d);
bool direction_validate(direction_t d);
direction_t direction_negotiate(direction_t r, direction_t c, bool *valid);
const char *packet_name(packet_t c);
bool packet_validate(packet_t p);
packet_t packet_throttle(packet_t request, packet_t limit);
packet_t packet_negotiate(packet_t request, packet_t c, bool *valid);
uint32_t packet_bytes(packet_t p);
const char *throughput_name(throughput_t c);
bool throughput_validate(throughput_t p);
throughput_t throughput_throttle(throughput_t request, throughput_t limit);
throughput_t throughput_negotiate(throughput_t r, throughput_t c, bool *valid);
uint32_t throughput_bps(throughput_t p);
bool window_validate(uint16_t i);
uint16_t window_throttle(uint16_t request, uint16_t limit);
uint16_t window_negotiate(uint16_t request, uint16_t current, bool *valid);





/*  These are the joza_msg messages
    DATA - Binary data message.
        q             number 1
        pr            number 2
        ps            number 2
        data          frame
    RR - Tells peer the lowest send sequence number that it may send in its DATA packet.
        pr            number 2
    RNR - Tells peer to stop sending data.
        pr            number 2
    CALL_REQUEST - Call a peer. Negotiate the type of connection requested.
        calling_address  string
        called_address  string
        packet        number 1
        window        number 2
        throughput    number 1
        data          frame
    CALL_ACCEPTED - Answer the call.
        calling_address  string
        called_address  string
        packet        number 1
        window        number 2
        throughput    number 1
        data          frame
    CLEAR_REQUEST - Request call termination
        cause         number 1
        diagnostic    number 1
    CLEAR_CONFIRMATION - Accept call termination
    RESET_REQUEST - Tell the peer to restart flow control
        cause         number 1
        diagnostic    number 1
    RESET_CONFIRMATION - Tell the peer that we have restarted flow control
    CONNECT - Client node requests connection to the broker.
        protocol      string
        version       number 1
        calling_address  string
        directionality  number 1
    CONNECT_INDICATION - Broker tells node that it has been connected.
    DISCONNECT - Node tells broker that it is disconnecting.
    DISCONNECT_INDICATION - Broker tells node that it has been disconnected
    DIAGNOSTIC - This is the Switched Virtual Call protocol version 1
        cause         number 1
        diagnostic    number 1
    COUNT - This is the Switched Virtual Call protocol version 1
*/

#define JOZA_MSG_VERSION                    1

#define JOZA_MSG_DATA                       0
#define JOZA_MSG_RR                         1
#define JOZA_MSG_RNR                        2
#define JOZA_MSG_CALL_REQUEST               3
#define JOZA_MSG_CALL_ACCEPTED              4
#define JOZA_MSG_CLEAR_REQUEST              5
#define JOZA_MSG_CLEAR_CONFIRMATION         6
#define JOZA_MSG_RESET_REQUEST              7
#define JOZA_MSG_RESET_CONFIRMATION         8
#define JOZA_MSG_CONNECT                    9
#define JOZA_MSG_CONNECT_INDICATION         10
#define JOZA_MSG_DISCONNECT                 11
#define JOZA_MSG_DISCONNECT_INDICATION      12
#define JOZA_MSG_DIAGNOSTIC                 13
#define JOZA_MSG_COUNT                      14

//  Structure of our class

typedef struct _joza_msg_t {
    zframe_t *address;          //  Address of peer if any
    int id;                     //  joza_msg message ID
    byte *needle;               //  Read/write pointer for serialization
    byte *ceiling;              //  Valid upper limit for read pointer
    byte q;
    uint16_t pr;
    uint16_t ps;
    zframe_t *data;
    char *calling_address;
    char *called_address;
    byte packet;
    uint16_t window;
    byte throughput;
    byte cause;
    byte diagnostic;
    char *protocol;
    byte version;
    byte directionality;
} joza_msg_t;


//  @interface
//  Create a new joza_msg
joza_msg_t *
    joza_msg_new (int id);

//  Destroy the joza_msg
void
    joza_msg_destroy (joza_msg_t **self_p);

//  Receive and parse a joza_msg from the input
joza_msg_t *
    joza_msg_recv (void *input);

//  Send the joza_msg to the output, and destroy it
int
    joza_msg_send (joza_msg_t **self_p, void *output);

//  Send the DATA to the output in one step
int
    joza_msg_send_data (void *output,
        byte q,
        uint16_t pr,
        uint16_t ps,
        zframe_t *data);

int
    joza_msg_send_addr_data (void *output, const zframe_t *addr,
        byte q,
        uint16_t pr,
        uint16_t ps,
        zframe_t *data);

//  Send the RR to the output in one step
int
    joza_msg_send_rr (void *output,
        uint16_t pr);

int
    joza_msg_send_addr_rr (void *output, const zframe_t *addr,
        uint16_t pr);

//  Send the RNR to the output in one step
int
    joza_msg_send_rnr (void *output,
        uint16_t pr);

int
    joza_msg_send_addr_rnr (void *output, const zframe_t *addr,
        uint16_t pr);

//  Send the CALL_REQUEST to the output in one step
int
    joza_msg_send_call_request (void *output,
        char *calling_address,
        char *called_address,
        byte packet,
        uint16_t window,
        byte throughput,
        zframe_t *data);

int
    joza_msg_send_addr_call_request (void *output, const zframe_t *addr,
        char *calling_address,
        char *called_address,
        byte packet,
        uint16_t window,
        byte throughput,
        zframe_t *data);

//  Send the CALL_ACCEPTED to the output in one step
int
    joza_msg_send_call_accepted (void *output,
        char *calling_address,
        char *called_address,
        byte packet,
        uint16_t window,
        byte throughput,
        zframe_t *data);

int
    joza_msg_send_addr_call_accepted (void *output, const zframe_t *addr,
        char *calling_address,
        char *called_address,
        byte packet,
        uint16_t window,
        byte throughput,
        zframe_t *data);

//  Send the CLEAR_REQUEST to the output in one step
int
    joza_msg_send_clear_request (void *output,
        byte cause,
        byte diagnostic);

int
    joza_msg_send_addr_clear_request (void *output, const zframe_t *addr,
        byte cause,
        byte diagnostic);

//  Send the CLEAR_CONFIRMATION to the output in one step
int
    joza_msg_send_clear_confirmation (void *output);

int
    joza_msg_send_addr_clear_confirmation (void *output, const zframe_t *addr);

//  Send the RESET_REQUEST to the output in one step
int
    joza_msg_send_reset_request (void *output,
        byte cause,
        byte diagnostic);

int
    joza_msg_send_addr_reset_request (void *output, const zframe_t *addr,
        byte cause,
        byte diagnostic);

//  Send the RESET_CONFIRMATION to the output in one step
int
    joza_msg_send_reset_confirmation (void *output);

int
    joza_msg_send_addr_reset_confirmation (void *output, const zframe_t *addr);

//  Send the CONNECT to the output in one step
int
    joza_msg_send_connect (void *output,
        char *calling_address,
        direction_t directionality);

int
    joza_msg_send_addr_connect (void *output, const zframe_t *addr,
        char *calling_address,
        byte directionality);

//  Send the CONNECT_INDICATION to the output in one step
int
    joza_msg_send_connect_indication (void *output);

int
    joza_msg_send_addr_connect_indication (void *output, const zframe_t *addr);

//  Send the DISCONNECT to the output in one step
int
    joza_msg_send_disconnect (void *output);

int
    joza_msg_send_addr_disconnect (void *output, const zframe_t *addr);

//  Send the DISCONNECT_INDICATION to the output in one step
int
    joza_msg_send_disconnect_indication (void *output);

int
    joza_msg_send_addr_disconnect_indication (void *output, const zframe_t *addr);

//  Send the DIAGNOSTIC to the output in one step
int
    joza_msg_send_diagnostic (void *output,
        byte cause,
        byte diagnostic);

int
    joza_msg_send_addr_diagnostic (void *output, const zframe_t *addr,
        byte cause,
        byte diagnostic);

//  Send the COUNT to the output in one step
int
    joza_msg_send_count (void *output);

int
    joza_msg_send_addr_count (void *output, const zframe_t *addr);

//  Duplicate the joza_msg message
joza_msg_t *
    joza_msg_dup (joza_msg_t *self);

//  Print contents of message to stdout
void
    joza_msg_dump (joza_msg_t *self);

//  Get/set the message address
zframe_t *
    joza_msg_address (joza_msg_t *self);
const zframe_t *
    joza_msg_const_address (const joza_msg_t *self);
void
    joza_msg_set_address (joza_msg_t *self, zframe_t *address);

//  Get the joza_msg id and printable command
int
    joza_msg_id (joza_msg_t *self);
int
    joza_msg_const_id (const joza_msg_t *self);
void
    joza_msg_set_id (joza_msg_t *self, int id);
const char *
    joza_msg_const_command (const joza_msg_t *self);

//  Get/set the q field
byte
    joza_msg_q (joza_msg_t *self);
byte
    joza_msg_const_q (const joza_msg_t *self);
void
    joza_msg_set_q (joza_msg_t *self, byte q);

//  Get/set the pr field
uint16_t
    joza_msg_pr (joza_msg_t *self);
uint16_t
    joza_msg_const_pr (const joza_msg_t *self);
void
    joza_msg_set_pr (joza_msg_t *self, uint16_t pr);

//  Get/set the ps field
uint16_t
    joza_msg_ps (joza_msg_t *self);
uint16_t
    joza_msg_const_ps (const joza_msg_t *self);
void
    joza_msg_set_ps (joza_msg_t *self, uint16_t ps);

//  Get/set the data field
zframe_t *
    joza_msg_data (joza_msg_t *self);
const zframe_t *
    joza_msg_const_data (const joza_msg_t *self);
void
    joza_msg_set_data (joza_msg_t *self, zframe_t *frame);

//  Get/set the calling_address field
char *
    joza_msg_calling_address (joza_msg_t *self);
const char *
    joza_msg_const_calling_address (const joza_msg_t *self);
void
    joza_msg_set_calling_address (joza_msg_t *self, const char *format, ...);

//  Get/set the called_address field
char *
    joza_msg_called_address (joza_msg_t *self);
const char *
    joza_msg_const_called_address (const joza_msg_t *self);
void
    joza_msg_set_called_address (joza_msg_t *self, const char *format, ...);

//  Get/set the packet field
byte
    joza_msg_packet (joza_msg_t *self);
byte
    joza_msg_const_packet (const joza_msg_t *self);
void
    joza_msg_set_packet (joza_msg_t *self, byte packet);

//  Get/set the window field
uint16_t
    joza_msg_window (joza_msg_t *self);
uint16_t
    joza_msg_const_window (const joza_msg_t *self);
void
    joza_msg_set_window (joza_msg_t *self, uint16_t window);

//  Get/set the throughput field
byte
    joza_msg_throughput (joza_msg_t *self);
byte
    joza_msg_const_throughput (const joza_msg_t *self);
void
    joza_msg_set_throughput (joza_msg_t *self, byte throughput);

//  Get/set the cause field
byte
    joza_msg_cause (joza_msg_t *self);
byte
    joza_msg_const_cause (const joza_msg_t *self);
void
    joza_msg_set_cause (joza_msg_t *self, byte cause);

//  Get/set the diagnostic field
byte
    joza_msg_diagnostic (joza_msg_t *self);
byte
    joza_msg_const_diagnostic (const joza_msg_t *self);
void
    joza_msg_set_diagnostic (joza_msg_t *self, byte diagnostic);

//  Get/set the directionality field
byte
    joza_msg_directionality (joza_msg_t *self);
byte
    joza_msg_const_directionality (const joza_msg_t *self);
void
    joza_msg_set_directionality (joza_msg_t *self, byte directionality);

//  Self test of this class
int
    joza_msg_test (bool verbose);
//  @end

