#pragma once
#include <glib.h>
#include <gio/gio.h>
#include "iodir.h"
#include "msg.h"

// To avoid circular dependency
typedef struct _JzChannel JzChannel;

typedef enum {
	C1_UNINITIALIZED = 0,
	C2_CLIENT_RESTART_REQUEST,
	C3_SERVER_RESTART_REQUEST,
    C4_INITIALIZED
} client_state_t;

#define CLIENT_RECV_BUFFER_SIZE (1024)

#define JZ_CLIENT_TIMER_RESTART_INDICATION_DURATION_IN_SECONDS (6)

/**
 * @brief List of possible active timers for a channel
 */
typedef enum {
    JZ_CLIENT_TIMER_NONE = 0,            /**< no active timer */
	JZ_CLIENT_TIMER_RESTART_INDICATION = 1
} JzClientTimerType;


struct _JzClient {
  GSocketConnection *connection;
  gboolean has_connection;

  // Text representation of the client's IP address
  char *inet_address_str;
  guint16 port;

  // Registration of the socket's input stream as a main loop source
  GSource *input_stream_source;

  // Connection state
  client_state_t state;
  gchar *address;
  gchar *hostname;
  iodir_t iodir;
  
  // Timers IDs, like from g_timeout_add
  JzClientTimerType timer_type;
  gint64 timer_expiration_time;
  int timer_count;

  // The input buffer
  guint8 recv_buffer[1024];
  gsize recv_buffer_size;

  // The input message queue of unprocessed messages
  GQueue *incoming;

  // The list of 
  GHashTable *channels;
};

typedef struct _JzClient JzClient;

// Creates a new JzClient, freshly allocated.
JzClient *jz_client_new (void);

// Frees the memory allocated for the JzClient. Only call this
// function if queue was created with jz_client_new().
void jz_client_free (JzClient *x);

// Attaches a socket connection to a JzClient so as to be managed as
// part of a connection to a client.
void jz_client_take_socket_connection (JzClient *C, GSocketConnection *S);

JzClient *jz_client_list_find_client (gchar *address);

JzChannel *jz_client_reserve_low_channel (JzClient *C);

void
jz_client_send_msg (JzClient *C, JzMsg *M);

void
jz_client_send_then_free_msg (JzClient *C, JzMsg *M);

void jz_client_check_timers (JzClient *C);
