#pragma once
#include <glib.h>
#include "diag.h"
#include "cause.h"
#include "tput.h"
#include "packet.h"

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

    SABM aka CONNECT - Client node registers connection to the broker.
    DISC aka DISCONNECT - Client node disconnects from the server
    UA aka ACKNOWLEDGEMENT - Server responds to CONNECT or DISCONNECT
    DM aka DISCONNECTED_MODE - Server's response to any message when in DISCONNECTED mode


    CONNECT_INDICATION - Broker tells node that it has been connected.
    DISCONNECT - Node tells broker that it is disconnecting.
    DISCONNECT_INDICATION - Broker tells node that it has been disconnected
    DIAGNOSTIC - A warning message
        cause         number 1
        diagnostic    number 1
    DIRECTORY_REQUEST - Node requests current list of users from Broker
    DIRECTORY - A dictionary list of users
        workers       dictionary
    ENQ - Are you still there?
    ACK - I am still here
*/

#define JZ_MSG_VERSION                    1
#define JZ_MSG_COUNT                      20

#define JZ_MSG_DATA                       0
#define JZ_MSG_RR                         1
#define JZ_MSG_RNR                        2
#define JZ_MSG_CALL_REQUEST               3
#define JZ_MSG_CALL_ACCEPTED              4
#define JZ_MSG_CLEAR_REQUEST              5
#define JZ_MSG_CLEAR_CONFIRMATION         6
#define JZ_MSG_RESET_REQUEST              7
#define JZ_MSG_RESET_CONFIRMATION         8
#define JZ_MSG_CONNECT                    9
#define JZ_MSG_CONNECT_INDICATION         10
#define JZ_MSG_DISCONNECT                 11
#define JZ_MSG_DISCONNECT_INDICATION      12
#define JZ_MSG_DIAGNOSTIC                 13
#define JZ_MSG_DIRECTORY_REQUEST          14
#define JZ_MSG_DIRECTORY                  15
#define JZ_MSG_ENQ                        16
#define JZ_MSG_ACK                        17
#define JZ_MSG_RESTART_REQUEST            18
#define JZ_MSG_RESTART_CONFIRMATION       19

#define JZ_MSG_ERROR_NONE                    0x0000
#define JZ_MSG_ERROR_INVALID_SIGNATURE       0x0001
#define JZ_MSG_ERROR_UNSUPPORTED_VERSION     0x0002
#define JZ_MSG_ERROR_UNKNOWN_MSG_ID          0x0004
#define JZ_MSG_ERROR_INVALID_CALLED_ADDRESS  0x0008
#define JZ_MSG_ERROR_INVALID_PACKET          0x0010
#define JZ_MSG_ERROR_INVALID_THROUGHPUT      0x0020
#define JZ_MSG_ERROR_INVALID_WINDOW          0x0040
#define JZ_MSG_ERROR_INVALID_CALLING_ADDRESS 0x0080
#define JZ_MSG_ERROR_INVALID_DATA_SIZE       0x0100
#define JZ_MSG_ERROR_INVALID_CAUSE           0x0200
#define JZ_MSG_ERROR_INVALID_DIAGNOSTIC      0x0400
#define JZ_MSG_ERROR_INVALID_Q               0x0800
#define JZ_MSG_ERROR_INVALID_PR              0x1000
#define JZ_MSG_ERROR_INVALID_PS              0x2000
#define JZ_MSG_ERROR_INVALID_IODIR           0x4000
#define JZ_MSG_ERROR_INVALID_HOSTNAME        0x8000

#define JZ_MSG_ENVELOPE_SIGNATURE_SIZE 4
#define JZ_MSG_ENVELOPE_HEADER_SIZE (JZ_MSG_ENVELOPE_SIGNATURE_SIZE + 4)
#define JZ_MSG_ENVELOPE_FOOTER_SIZE 4
#define JZ_MSG_ENVELOPE_SIZE (JZ_MSG_ENVELOPE_HEADER_SIZE + JZ_MSG_ENVELOPE_FOOTER_SIZE)
#define JZ_MSG_PAYLOAD_HEADER_SIZE 4
#define JZ_MSG_PADDING_LENGTH(__body_size) ((__body_size) % 4 ? (4 - ((__body_size) % 4)) : 0)

#define JZ_MSG_MAX_DATA_SIZE 1024
#define JZ_MSG_MAX_CALL_REQUEST_DATA_SIZE 16
#define JZ_MSG_MAX_PAYLOAD_LENGTH (JZ_MSG_MAX_DATA_SIZE + 8)

#define JZ_MSG_MAX_ADDRESS_LENGTH 16
#define JZ_MSG_MAX_HOSTNAME_LENGTH 80
#define JZ_MSG_MAX_LCN 1024

struct _JzMsg {
  // Header for all messages
  guint32 signature;
  guint8 version;
  guint8 id;

  // Bookkeeping
  guint8 *needle;               //  Read/write pointer for serialization
  guint8 *ceiling;              //  Valid upper limit for read pointer
  gsize packed_size;

  // Validity
  gboolean valid;
  diag_t invalidity;           // If invalid, the last problem found with the message

  // Connections 
  gchar *hostname;
  guint8 iodir;

  // Directory messages
  gchar *directory;

  // Calls
  guint16 lcn;                  // Logical channel number
  gchar *calling_address;
  gchar *called_address;
  guint8 packet;
  guint16 window;
  guint8 throughput;

  // Diagnostic
  guint8 cause;
  guint8 diagnostic;
  guint8 diagnostic_version;
  guint8 diagnostic_id;
  guint16 diagnostic_lcn;

  // Data and flow control
  guint8 q;
  guint16 pr;
  guint16 ps;
  GByteArray *data;

  guint32 crc;
};

typedef struct _JzMsg JzMsg;

#define JZ_MSG_SIGNATURE 0x7e535643   // as a string "~SVC"

const char *id_name(guint8 id);

JzMsg *jz_msg_new_from_data (gpointer buf, gsize len);
gboolean jz_msg_is_valid (JzMsg *M);
gsize jz_msg_binary_size (JzMsg *M);
void jz_msg_free (JzMsg *M);
diag_t jz_msg_validate (JzMsg *M);

GByteArray *jz_msg_to_byte_array (JzMsg *self);

JzMsg *jz_msg_new_connect (gchar *address, guint8 iodir);
JzMsg *jz_msg_new_connect_indication ();
JzMsg *jz_msg_new_disconnect ();
JzMsg *jz_msg_new_disconnect_indication ();
JzMsg * jz_msg_new_call_request (guint16 lcn, gchar *calling_address, gchar *called_address, packet_t packet,
                                 guint16 window, tput_t throughput, GByteArray *arr);
JzMsg * jz_msg_new_call_accepted (guint16 lcn, gchar *calling_address, gchar *called_address, packet_t packet,
                                  guint16 window, tput_t throughput, GByteArray *arr);
JzMsg *jz_msg_new_clear_request (guint16 lcn, cause_t cause, diag_t diagnostic);
JzMsg *jz_msg_new_clear_confirmation (guint16 lcn);
JzMsg *jz_msg_new_diagnostic (diag_t diagnostic, guint8 version, guint8 id, guint8 lcn);
JzMsg *jz_msg_new_restart_request (cause_t cause, diag_t diagnostic);
JzMsg *jz_msg_new_restart_confirmation ();

gboolean jz_msg_is_for_channel (JzMsg *M);

gboolean jz_buffer_begins_with_signature (guint8 *buf, gsize len);
gboolean jz_buffer_contains_a_message (guint8 *buf, gsize len);
gboolean jz_buffer_msg_envelope_is_valid (guint8 *buf, gsize len);
guint32 jz_buffer_msg_body_length (guint8 *buf, gsize len);
guint16 jz_buffer_msg_lcn (guint8 *buf, gsize len);



