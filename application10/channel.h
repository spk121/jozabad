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
  P7_SERVER_CLEAR_REQUEST,
  D1_FLOW_CONTROL_READY,
  D2_CLIENT_RESET_REQUEST,
  D3_SERVER_RESET_REQUEST
} channel_state_t;

struct _JzChannel {
  guint16 lcn;
  JzClient *parent;

  // This is the connection to another channel
  struct _JzChannel *peer;
  
  channel_state_t state;
  guint16 window;
  guint8 throughput;
  guint8 packet;
  guint8 max_packet_size;
};

typedef struct _JzChannel JzChannel;

// Creates a new JzChannel, freshly allocated
JzChannel *jz_channel_new (JzClient *client);

// Frees the memory allocated for the JzChannel
void jz_channel_free (JzChannel *C);

void jz_channel_process_message (JzChannel *C, JzMsg *msg);

void jz_channel_do_client_restart_request (JzChannel *X, diag_t diagnostic);
void jz_channel_do_server_restart_request (JzChannel *X, diag_t diagnostic);


  
