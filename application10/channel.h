#pragma once
#include <glib.h>
#include "client.h"
#include "msg.h"

typedef enum {
  P1_READY,
  P2_CLIENT_WAITING,
  P3_SERVER_WAITING,
  P5_CALL_COLLISION,
  P6_CLIENT_CLEAR_REQUEST,
  P7_SERVER_CLEAR_INDICATION,
  D1_FLOW_CONTROL_READY,
  D2_CLIENT_RESET_REQUEST,
  D3_SERVER_RESET_INDICATION
} channel_state_t;

static char short_channel_state_name[9][3] = {
  [P1_READY] = "P1",
  [P2_CLIENT_WAITING] = "P2",
  [P3_SERVER_WAITING] = "P3",
  [P5_CALL_COLLISION] = "P5",
  [P6_CLIENT_CLEAR_REQUEST] = "P6",
  [P7_SERVER_CLEAR_INDICATION] = "P7",
  [D1_FLOW_CONTROL_READY] = "D1",
  [D2_CLIENT_RESET_REQUEST] = "D2",
  [D3_SERVER_RESET_INDICATION] = "D3",
};


/**
 * @brief The number of seconds before the incoming call timer expires
 */
#define TIMER_INCOMING_CALL_DURATION_IN_SECONDS (UINT64_C(18))
/**
 * @brief The number of iterations the incoming call timer before it expires
*/
#define TIMER_CALL_INDICATION_ITERATIONS (1)
/**
 * @brief The number of seconds per iteration of the reset-indication timer
 */
#define TIMER_RESET_INDICATION_DURATION_IN_SECONDS (UINT64_C(6))
/**
 * @brief The number of iterations the reset-indication timer before it expires
*/
#define TIMER_RESET_INDICATION_ITERATIONS (2)
/**
 * @brief The number of seconds per iteration of the clear-indication timer
 */
#define TIMER_CLEAR_INDICATION_DURATION_IN_SECONDS (UINT64_C(6))
/**
 * @brief The number of iterations the clear-indication timer before it expires
*/
#define TIMER_CLEAR_INDICATION_ITERATIONS (2)

/**
 * @brief List of possible active timers for a channel
 */
typedef enum {
    TIMER_NONE = 0,            /**< no active timer */
	TIMER_INCOMING_CALL = 1,
	TIMER_RESET_INDICATION = 2,
	TIMER_CLEAR_INDICATION = 3
} JzChannelTimerType;

struct _JzChannel {
  guint16 lcn;
  JzClient *parent;

  // This is the connection to another channel
  struct _JzChannel *peer;
  
  channel_state_t state;

  // FLOW CONTROL
  guint16 client_ps;                   /**< ID of the next packet channel expects to receive from client */
  guint16 client_pr;                   /**< Smallest packet ID that server may send to client */
  guint16 peer_ps;                   /**< ID of the next packet channel expects to receive from client */
  guint16 peer_pr;                   /**< Smallest packet ID that server may send to client */
  guint16 window;               /**< delta between smallest and largest acceptable ID */

  // LIMITS
  guint8 throughput;            /**< Throughput allowed for this channel */
  guint8 packet;                /**< Packet size (enum) allowed on this channel */
  
  guint8 max_packet_size;

  // TIMERS
  JzChannelTimerType timer_type;  /**< The type of active timer set for this channel */
  gint64 timer_expiration_time;     /**< expiration time */
  int timer_count;  /**< the number of times this timer has expired */
  
};

typedef struct _JzChannel JzChannel;

// Creates a new JzChannel, freshly allocated
JzChannel *jz_channel_new (JzClient *client, guint16 lcn);

// Frees the memory allocated for the JzChannel
void jz_channel_free (JzChannel *C);

void jz_channel_process_message (JzChannel *C, JzMsg *msg);

void jz_channel_do_client_restart_request (JzChannel *X, diag_t diagnostic);
void jz_channel_do_server_restart_indication (JzChannel *X, diag_t diagnostic);

void jz_channel_check_timers_wrapper (gpointer key, gpointer value, gpointer user_data);

  
