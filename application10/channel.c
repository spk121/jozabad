#include <stdio.h>
#include "channel.h"
#include "seq.h"

typedef void (*action_func_t)(JzChannel *X, JzMsg *msg);


void jz_channel_process_message_in_P1_ready_state (JzChannel *X, JzMsg *msg);
void jz_channel_process_message_in_P2_client_waiting_state (JzChannel *X, JzMsg *msg);
void jz_channel_process_message_in_P3_server_waiting_state (JzChannel *X, JzMsg *msg);
void jz_channel_process_message_in_D1_flow_control_ready_state (JzChannel *X, JzMsg *msg);
void jz_channel_process_message_in_D2_client_reset_request_state (JzChannel *X, JzMsg *msg);
void jz_channel_process_message_in_D3_server_reset_indication_state (JzChannel *X, JzMsg *msg);
void jz_channel_process_message_in_P5_call_collision_state (JzChannel *X, JzMsg *msg);
void jz_channel_process_message_in_P6_client_clear_request_state (JzChannel *X, JzMsg *msg);
void jz_channel_process_message_in_P7_server_clear_indication_state (JzChannel *X, JzMsg *msg);

void jz_channel_process_server_message (JzChannel *C, JzMsg *msg);
void jz_channel_process_server_message_in_P1_ready_state (JzChannel *X, JzMsg *msg);
void jz_channel_process_server_message_in_P2_client_waiting_state (JzChannel *X, JzMsg *msg);
void jz_channel_process_server_message_in_P3_server_waiting_state (JzChannel *X, JzMsg *msg);
void jz_channel_process_server_message_in_D1_flow_control_ready_state (JzChannel *X, JzMsg *msg);
void jz_channel_process_server_message_in_D2_client_reset_request_state (JzChannel *X, JzMsg *msg);
void jz_channel_process_server_message_in_D3_server_reset_indication_state (JzChannel *X, JzMsg *msg);
void jz_channel_process_server_message_in_P5_call_collision_state (JzChannel *X, JzMsg *msg);
void jz_channel_process_server_message_in_P6_client_clear_request_state (JzChannel *X, JzMsg *msg);
void jz_channel_process_server_message_in_P7_server_clear_indication_state (JzChannel *X, JzMsg *msg);


void jz_channel_do_client_call_request (JzChannel *X, JzMsg *msg);
void jz_channel_do_client_call_accepted (JzChannel *X, JzMsg *msg);
void jz_channel_do_client_call_collision (JzChannel *X, JzMsg *msg);
void jz_channel_do_client_clear_request (JzChannel *X, JzMsg *msg);
void jz_channel_do_client_clear_confirmation (JzChannel *X, JzMsg *msg);
void jz_channel_do_client_reset_request (JzChannel *X, JzMsg *msg);
void jz_channel_do_client_reset_confirmation (JzChannel *X, JzMsg *msg);
void jz_channel_do_client_reset_collision (JzChannel *X, JzMsg *msg);

void jz_channel_do_client_data (JzChannel *X, JzMsg *msg);
void jz_channel_do_client_rr (JzChannel *X, JzMsg *msg);
void jz_channel_do_client_rnr (JzChannel *X, JzMsg *msg);

void jz_channel_do_server_clear_indication (JzChannel *X, JzMsg *msg);
void jz_channel_do_server_clear_confirmation (JzChannel *X, JzMsg *msg);
void jz_channel_do_server_incoming_call (JzChannel *X, JzMsg *msg);
void jz_channel_do_server_call_connected (JzChannel *X, JzMsg *msg);
void jz_channel_do_server_call_collision (JzChannel *X, JzMsg *msg);
void jz_channel_do_server_reset_indication (JzChannel *X, JzMsg *msg);
void jz_channel_do_server_reset_confirmation (JzChannel *X, JzMsg *msg);
void jz_channel_do_server_data (JzChannel *X, JzMsg *msg);
void jz_channel_do_server_rr (JzChannel *X, JzMsg *msg);
void jz_channel_do_server_rnr (JzChannel *X, JzMsg *msg);

void jz_channel_do_punishment_clear_request (JzChannel *X, clear_cause_t cause, diag_t diagnostic);
void jz_channel_do_punishment_reset_request (JzChannel *X, reset_cause_t cause, diag_t diagnostic);

void jz_channel_timer_reset (JzChannel *X);

void jz_channel_check_timers (JzChannel *X);
void jz_channel_do_incoming_call_expiration (JzChannel *X);
void jz_channel_do_reset_indication_expiration (JzChannel *X);
void jz_channel_do_clear_indication_expiration (JzChannel *X);


#define COPY_BYTE_ARRAY(__X) g_byte_array_new_take (g_memdup((__X)->data, (__X)->len), (__X)->len)

JzChannel *jz_channel_new (JzClient *client, guint16 lcn)
{
  JzChannel *C = g_new0 (JzChannel, 1);
  C->parent = client;
  C->peer = NULL;
  C->state = P1_READY;
  C->lcn = lcn;
  C->timer_type = TIMER_NONE;
  return C;
}

void jz_channel_free (JzChannel *C)
{
  C->parent = NULL;
  if (C->peer)
  {
    C->peer->peer = NULL;
    C->peer = NULL;
  }
  C->state = P1_READY;
  g_free (C);
}

void jz_channel_dump (JzChannel *C)
{
  printf("CHANNEL %s:%d->%s:%d STATE %s CLIENT PS %d PR %d PEER PS %d PR %d\n",
         C->parent->address,
         (int) C->lcn,
         C->peer ? C->peer->parent->address : "NONE",
         C->peer ? (int) C->peer->lcn : 0,
         short_channel_state_name[C->state],
         C->client_ps, C->client_pr, C->peer_ps, C->peer_pr);
}

void jz_channel_process_message (JzChannel *C, JzMsg *msg)
{
  channel_state_t original_state = C->state;
  
  switch (original_state) {
  case P1_READY:
    jz_channel_process_message_in_P1_ready_state (C, msg);
    break;
  case P2_CLIENT_WAITING:
    jz_channel_process_message_in_P2_client_waiting_state (C, msg);
    break;
  case P3_SERVER_WAITING:
    jz_channel_process_message_in_P3_server_waiting_state (C, msg);
    break;
  case D1_FLOW_CONTROL_READY:
    jz_channel_process_message_in_D1_flow_control_ready_state (C, msg);
    break;
  case D2_CLIENT_RESET_REQUEST:
    jz_channel_process_message_in_D2_client_reset_request_state (C, msg);
    break;
  case D3_SERVER_RESET_INDICATION:
    jz_channel_process_message_in_D3_server_reset_indication_state (C, msg);
    break;
  case P5_CALL_COLLISION:
    jz_channel_process_message_in_P5_call_collision_state (C, msg);
    break;
  case P6_CLIENT_CLEAR_REQUEST:
    jz_channel_process_message_in_P6_client_clear_request_state (C, msg);
    break;
  case P7_SERVER_CLEAR_INDICATION:
    jz_channel_process_message_in_P7_server_clear_indication_state (C, msg);
    break;
  }

  jz_channel_dump (C);
}

void jz_channel_process_message_in_P1_ready_state (JzChannel *X, JzMsg *msg)
{
  if (msg->id == JZ_MSG_CLEAR_REQUEST)
    jz_channel_do_client_clear_request (X, msg);
  
  else if (msg->id == JZ_MSG_CALL_REQUEST)
    jz_channel_do_client_call_request (X, msg);
  else
    jz_channel_do_punishment_clear_request (X, c_clear__local_procedure_error, d_invalid_message_for_state_ready);
}

void jz_channel_process_message_in_P2_client_waiting_state (JzChannel *X, JzMsg *msg)
{
  if (msg->id == JZ_MSG_CLEAR_REQUEST)
    jz_channel_do_client_clear_request (X, msg);
  
  else
    jz_channel_do_punishment_clear_request (X, c_clear__local_procedure_error, d_invalid_message_for_state_client_waiting);
}

void jz_channel_process_message_in_P3_server_waiting_state (JzChannel *X, JzMsg *msg)
{
  channel_state_t original_state = X->state;

  if (msg->id == JZ_MSG_CLEAR_REQUEST)
    jz_channel_do_client_clear_request (X, msg);
  
  else if (msg->id == JZ_MSG_CALL_REQUEST)
    jz_channel_do_client_call_collision (X, msg);
  else if (msg->id == JZ_MSG_CALL_ACCEPTED)
    jz_channel_do_client_call_accepted (X, msg);
  else
    jz_channel_do_punishment_clear_request (X, c_clear__local_procedure_error, d_invalid_message_for_state_server_waiting);

  if (X->state != original_state && X->timer_type == TIMER_INCOMING_CALL)
	jz_channel_timer_reset (X);
}

void jz_channel_process_message_in_D1_flow_control_ready_state (JzChannel *X, JzMsg *msg)
{
  if (msg->id == JZ_MSG_CLEAR_REQUEST)
    jz_channel_do_client_clear_request (X, msg);

  else if (msg->id == JZ_MSG_RESET_REQUEST)
    jz_channel_do_client_reset_request (X, msg);
  
  else if (msg->id == JZ_MSG_DATA)
    jz_channel_do_client_data (X, msg);
  else if (msg->id == JZ_MSG_RR)
    jz_channel_do_client_rr (X, msg);
  else if (msg->id == JZ_MSG_RNR)
    jz_channel_do_client_rnr (X, msg);
  else
    jz_channel_do_punishment_reset_request (X, c_reset__local_procedure_error, d_invalid_message_for_state_flow_control_ready);

}

void jz_channel_process_message_in_D2_client_reset_request_state (JzChannel *X, JzMsg *msg)
{
  if (msg->id == JZ_MSG_CLEAR_REQUEST)
    jz_channel_do_client_clear_request (X, msg);

  else if (msg->id == JZ_MSG_RESET_REQUEST)
    // ignore
    ;
  else
    jz_channel_do_punishment_reset_request (X, c_reset__local_procedure_error,
											d_invalid_message_for_state_client_reset_request);
}

void jz_channel_process_message_in_D3_server_reset_indication_state (JzChannel *X, JzMsg *msg)
{
  channel_state_t original_state = X->state;

  if (msg->id == JZ_MSG_CLEAR_REQUEST)
    jz_channel_do_client_clear_request (X, msg);

  else if (msg->id == JZ_MSG_RESET_REQUEST || msg->id == JZ_MSG_RESET_CONFIRMATION)
    jz_channel_do_client_reset_confirmation (X, msg);
  else
    // Ignore everything, especially DATA, RR, and RNR packets.
    ;

  if (X->state != original_state && X->timer_type == TIMER_RESET_INDICATION)
	jz_channel_timer_reset (X);

}

void jz_channel_process_message_in_P5_call_collision_state (JzChannel *X, JzMsg *msg)
{
  if (msg->id == JZ_MSG_CLEAR_REQUEST)
    jz_channel_do_client_clear_request (X, msg);
  else
    jz_channel_do_punishment_clear_request (X, c_clear__local_procedure_error, d_invalid_message_for_state_call_collision);
}

void jz_channel_process_message_in_P6_client_clear_request_state (JzChannel *X, JzMsg *msg)
{
  if (msg->id == JZ_MSG_CLEAR_REQUEST)
    // Discard
    ;
  else
    jz_channel_do_punishment_clear_request (X, c_clear__local_procedure_error, d_invalid_message_for_state_client_clear_request);
}

void jz_channel_process_message_in_P7_server_clear_indication_state (JzChannel *X, JzMsg *msg)
{
  channel_state_t original_state = X->state;

  if (msg->id == JZ_MSG_CLEAR_REQUEST || msg->id == JZ_MSG_CLEAR_CONFIRMATION)
    jz_channel_do_client_clear_confirmation (X, msg);
  else
    // DISCARD
    ;

  if (X->state != original_state && X->timer_type == TIMER_CLEAR_INDICATION)
	jz_channel_timer_reset (X);
}

void jz_channel_do_client_call_request (JzChannel *X, JzMsg *msg)
{
  clear_cause_t cause;
  diag_t diagnostic;
  JzClient *peer = NULL;
  JzChannel *Y = NULL;

  // UNIMPLEMENTED: Here X.25 would check if a particular LCN is a one-way incoming channel
  //  but we haven't chosen different behaviors for different LCNs yet.
  // Here we check if the client has outgoing calls barred.  N.B. This
  // is different from the unimplemented "one way logical channel"
  // above by being a function of all the channels in the client.
  if (X->parent->iodir == io_outgoing_calls_barred || X->parent->iodir == io_calls_barred)
    {
      cause = c_clear__local_procedure_error;
      diagnostic = d_outgoing_calls_barred;
      goto fail;
    }
  // Here X.25 would re-check the basic message validity, but, we did than when the
  // message was received.
  // X.25: check if address contains non-BCD digit
  // X.25: check in calling address is invalid
  // X.25: check if called address is invalid
  // X.25: check facility codes and values
  if (seq_rngchk (msg->window) != 0)
    {
      cause = c_clear__local_procedure_error;
      diagnostic = d_invalid_window_facility;
      goto fail;
    }
  else if (tput_rngchk (msg->throughput) != 0)
    {
      cause = c_clear__local_procedure_error;
      diagnostic = d_invalid_throughput_facility;
      goto fail;
    }
  else if (packet_rngchk (msg->packet) != 0)
    {
      cause = c_clear__local_procedure_error;
      diagnostic = d_invalid_packet_facility;
      goto fail;
    }

  // Here we check if there is a peer with the given name
  if ((peer = jz_client_list_find_client (msg->called_address)) == NULL)
    {
      cause = c_clear__not_obtainable;
      diagnostic = d_invalid_called_address;
      goto fail;
    }

  // Here we check if the called client accepts incoming calls
  if (peer->iodir == io_incoming_calls_barred || peer->iodir == io_calls_barred)
    {
      cause = c_clear__access_barred;
      diagnostic = d_input_barred;
      goto fail;
    }

  // Here we check if the called client has a free channel
  if ((Y = jz_client_reserve_low_channel (peer)) == NULL)
    {
      cause = c_clear__number_busy;
      diagnostic = d_no_logical_channel_available;
      goto fail;
    }

  // OK, make a connection
  X->peer = Y;
  X->peer->peer = X;

  // Store the facility requests for this channel
  X->window = msg->window;
  X->throughput = msg->throughput;
  X->packet = msg->packet;

  // Send the call request to the other peer's new channel
  JzMsg *msg2 = jz_msg_new_call_request (Y->lcn, msg->calling_address, msg->called_address,
                                         msg->packet, msg->window, msg->throughput,
                                         g_memdup (msg->data->data, msg->data->len), msg->data->len);
  jz_channel_process_server_message (Y, msg);
  jz_msg_free (msg2);

  // Start waiting for a reply
  X->state = P2_CLIENT_WAITING;

  return;

 fail:
  jz_channel_do_punishment_clear_request (X, cause, diagnostic);
}

void jz_channel_reset_flow_control (JzChannel *X)
{
  X->peer_ps = 0;
  X->peer_pr = 0;
  X->client_ps = 0;
  X->client_pr = 0;
  X->state = D1_FLOW_CONTROL_READY;
}

void jz_channel_do_client_call_accepted (JzChannel *X, JzMsg *msg)
{
  clear_cause_t cause;
  diag_t diagnostic;

  // FIXME: Check that the client hasn't cheated in the facility negotiation
  if (!window_negotiate (msg->window, X->window))
    {
      cause = c_clear__local_procedure_error;
      diagnostic = d_invalid_window_facility_negotiation;
      goto fail;
    }
  else if (!tput_negotiate (msg->throughput, X->throughput))
    {
      cause = c_clear__local_procedure_error;
      diagnostic = d_invalid_throughput_facility_negotiation;
      goto fail;
    }
  else if (!packet_negotiate (msg->packet, X->packet))
    {
      cause = c_clear__local_procedure_error;
      diagnostic = d_invalid_packet_facility_negotiation;
      goto fail;
    }

  X->window = msg->window;
  X->throughput = msg->throughput;
  X->packet = msg->packet;
  
  // Then, forward it on to the other side of the connection
  g_assert (X->peer);
  JzMsg *msgY = jz_msg_new_call_accepted (X->peer->lcn, msg->calling_address, msg->called_address,
                                          msg->packet, msg->window, msg->throughput,
										  g_memdup (msg->data->data, msg->data->len), msg->data->len);
  jz_channel_process_server_message (X->peer, msgY);
  jz_msg_free (msgY);
  jz_channel_reset_flow_control (X);

  return;
  
 fail:
  jz_channel_do_punishment_clear_request (X, cause, diagnostic);
}

void jz_channel_do_client_call_collision (JzChannel *X, JzMsg *msg)
{
  // FIXME: Check that the client hasn't cheated in the facility negotiation
  X->window = msg->window;
  X->throughput = msg->throughput;
  X->packet = msg->packet;
  
  // Then, forward it on to the other side of the connection
  g_assert (X->peer);
  JzMsg *msgY = jz_msg_new_call_request (X->peer->lcn, msg->calling_address, msg->called_address,
                                         msg->packet, msg->window, msg->throughput,
										 g_memdup (msg->data->data, msg->data->len), msg->data->len);
  jz_channel_process_server_message (X->peer, msgY);
  jz_msg_free (msgY);
  X->state = P5_CALL_COLLISION;
}


void jz_channel_do_client_clear_request (JzChannel *X, JzMsg *msg)
{
  // X.25: calling address length is not empty in the general case
  // X.25: called address length is not empty in the general case
  // X.25: user data is not empty in the general case

  // If this channel is connected, forward the clear request to Y
  if (X->peer)
    {
      JzMsg *msg2 = jz_msg_new_clear_request (X->peer->lcn, msg->cause, msg->diagnostic);
      jz_channel_process_server_message (X->peer, msg2);
      jz_msg_free (msg2);
    }

  // Start waiting for a reply
  X->state = P6_CLIENT_CLEAR_REQUEST;
}

void jz_channel_do_client_reset_request (JzChannel *X, JzMsg *msg)
{
  // If the reset request doesn't have CLIENT ORIGINATED as its cause,
  // that's an "improper cause code from client" error, which causes a
  // punishment reset with the cause "local procedure error".

  
  if (!CAUSE_IS_CLIENT_ORIGINATED (msg->cause))
	jz_channel_do_punishment_reset_request (X, c_reset__local_procedure_error, d_improper_cause_code_from_client);
  else
  {
    X->state = D2_CLIENT_RESET_REQUEST;

    if (X->peer)
    {
      JzMsg *msg2 = jz_msg_new_reset_request (X->peer->lcn, msg->cause, msg->diagnostic);
      jz_channel_process_server_message (X->peer, msg2);
      jz_msg_free (msg2);
    }
  }
}

void jz_channel_do_client_reset_confirmation (JzChannel *X, JzMsg *msg)
{
  jz_channel_reset_flow_control (X);

  if (X->peer)
    {
      JzMsg *msg2 = jz_msg_new_reset_confirmation (X->peer->lcn);
      jz_channel_process_server_message (X->peer, msg2);
      jz_msg_free (msg2);
    }
}

void jz_channel_do_client_reset_collision (JzChannel *X, JzMsg *msg)
{
  jz_channel_reset_flow_control (X);

  if (X->peer)
    {
      JzMsg *msg2 = jz_msg_new_reset_confirmation (X->peer->lcn);
      jz_channel_process_server_message (X->peer, msg2);
      jz_msg_free (msg2);
    }
}

void jz_channel_do_client_data (JzChannel *X, JzMsg *msg)
{
  diag_t diag;

  // The data payload is invalid if it is larger than the agreed-upon
  // packet size for this channel.
  if (msg->data->len >= packet_bytes(X->packet))
    {
      diag = d_packet_too_long;
      goto err;
    }

  // The data payload is invalid if it is empty
  else if (msg->data->len == 0)
    {
      diag = d_packet_too_short;
      goto err;
    }

  // When a client sends a message, the message's "packet send
  // sequence number" should be the next in the expected sequence and
  // within the window.
  else if ((msg->ps != X->client_ps)
           || (!seq_in_range (msg->ps, X->peer_pr, (X->peer_pr + X->window - 1) % SEQ_MAX)))
    {
      diag = d_invalid_ps;
      goto err;
    }

  // when a client updates its own window of packets that it will
  // accept, its new lowest packet number that it will allow should be
  // between the previous lowest packet number that it would allow and
  // the end of the window.
  else if (!seq_in_range (msg->pr, X->client_pr, X->peer_ps))
    {
      diag = d_pr_invalid_window_update;
      goto err;
    }
  else
    {
      // Increment the value of the next packet number expected from the client.
      X->client_ps = (X->client_ps + 1) % SEQ_MAX;
      X->client_pr = msg->pr;
      if (X->peer)
        {
          JzMsg *msg2 = jz_msg_new_data (X->peer->lcn, msg->q, msg->pr, msg->ps,
                                         g_memdup(msg->data->data, msg->data->len),
										 msg->data->len);
          jz_channel_process_server_message (X->peer, msg2);
          jz_msg_free (msg2);
        }
    }
  
  return;
  
 err:
  jz_channel_do_punishment_reset_request (X, c_reset__local_procedure_error, diag);
}

void jz_channel_do_client_rr (JzChannel *X, JzMsg *msg)
{
  diag_t diag;

  // When a client updates its own window of packets that it will
  // accept, its new lowest packet number that it will allow should be
  // between the previous lowest packet number that it would allow and
  // the end of the window.
  if (!seq_in_range (msg->pr, X->client_pr, X->peer_ps))
    {
      diag = d_pr_invalid_window_update;
      goto err;
    }
  else
    {
      X->client_pr = msg->pr;
      if (X->peer)
        {
          JzMsg *msg2 = jz_msg_new_rr (msg->lcn, msg->pr);
          jz_channel_process_server_message (X->peer, msg2);
          jz_msg_free (msg2);
        }
    }
  return;

 err:
  jz_channel_do_punishment_reset_request (X, c_reset__local_procedure_error, diag);
}

void jz_channel_do_client_rnr (JzChannel *X, JzMsg *msg)
{
  diag_t diag;

  // When a client updates its own window of packets that it will
  // accept, its new lowest packet number that it will allow should be
  // between the previous lowest packet number that it would allow and
  // the end of the window.
  if (!seq_in_range (msg->pr, X->client_pr, X->peer_ps))
    {
      diag = d_pr_invalid_window_update;
      goto err;
    }
  else
    {
      X->client_pr = msg->pr;
      if (X->peer)
        {
          JzMsg *msg2 = jz_msg_new_rnr (msg->lcn, msg->pr);
          jz_channel_process_server_message (X->peer, msg2);
          jz_msg_free (msg2);
        }
    }
  return;

 err:
  jz_channel_do_punishment_reset_request (X, c_reset__local_procedure_error, diag);
}



void jz_channel_do_punishment_clear_request (JzChannel *X, clear_cause_t cause, diag_t diagnostic)
{
  printf("CHANNEL %s:%d->%s:%d clear by server %s : %s\n",
         X->parent->address,
         (int) X->lcn,
         X->peer ? X->peer->parent->address : "NONE",
         X->peer ? (int) X->peer->lcn : 0,
         clear_cause_name (cause),
         diag_name (diagnostic));

  JzChannel *peer = X->peer;
  
  JzMsg *msgX = jz_msg_new_clear_request (X->lcn, cause, diagnostic);
  jz_channel_process_server_message (X, msgX);
  jz_msg_free (msgX);
  
  if (peer != NULL)
	{
  	  peer->peer = NULL;
	  
	  if (cause == c_clear__local_procedure_error)
		cause = c_clear__remote_procedure_error;
	  JzMsg *msgY = jz_msg_new_clear_request (peer->lcn, cause,
											  diagnostic);
	  jz_channel_process_server_message (peer, msgY);
	  jz_msg_free (msgY);
	}
}

void jz_channel_do_punishment_reset_request (JzChannel *X, reset_cause_t cause, diag_t diagnostic)
{
  printf("CHANNEL %s:%d->%s:%d reset by server %p : %p\n",
         X->parent->address,
         (int) X->lcn,
         X->peer ? X->peer->parent->address : "NONE",
         X->peer ? (int) X->peer->lcn : 0,
         reset_cause_name (cause),
         diag_name (diagnostic));

  JzChannel *peer = X->peer;
  
  JzMsg *msgX = jz_msg_new_reset_request (X->lcn, cause, diagnostic);
  jz_channel_process_server_message (X, msgX);
  jz_msg_free (msgX);

  if (peer != NULL)
    {
      if (cause == c_reset__local_procedure_error)
        cause = c_reset__remote_procedure_error;
      JzMsg *msgY = jz_msg_new_reset_request (peer->lcn, cause,
                                             diagnostic);
      jz_channel_process_server_message (peer, msgY);
	  jz_msg_free (msgY);
    }
}

void jz_channel_do_client_clear_confirmation (JzChannel *X, JzMsg *msg)
{
  // In the nominal case, the logical channel is set to READY.
  X->state = P1_READY;
}

// X has requested a RESTART, so for each channel on X, it is set to the READY state,
// disconnected from the channel, and the peer is sent a server clear request
void jz_channel_do_client_restart_request (JzChannel *X, diag_t diagnostic)
{
  JzChannel *Y = X->peer;

  X->peer = NULL;

  if (Y)
    {
	  Y->peer = NULL;
      JzMsg *msgY = jz_msg_new_clear_request (Y->lcn, c_clear__client_originated, diagnostic);
      jz_channel_process_server_message (Y, msgY);
      jz_msg_free (msgY);
    }

  X->state = P1_READY;
  jz_channel_timer_reset (X);
}

// X has goofed, and the server is demanding a RESTART, so for each
// channel on X, it is set to the READY state, disconnected from the
// channel, and the peer is sent a server clear request
void jz_channel_do_server_restart_indication (JzChannel *X, diag_t diagnostic)
{
  JzChannel *Y = X->peer;

  X->peer = NULL;

  if (Y)
    {
	  Y->peer = NULL;
      JzMsg *msgY = jz_msg_new_clear_request (Y->lcn, c_clear__remote_procedure_error,
                                             diagnostic);
      jz_channel_process_server_message (Y, msgY);
      jz_msg_free (msgY);
    }

  X->state = P1_READY;
  jz_channel_timer_reset (X);
}


////////////////////////////////////////////////////////////////
// INTRA-CONNECTION MESSAGES or SERVER-ORIGINATED MESSAGES
////////////////////////////////////////////////////////////////

void jz_channel_process_server_message (JzChannel *C, JzMsg *msg)
{
  switch (C->state) {
  case P1_READY:
    jz_channel_process_server_message_in_P1_ready_state (C, msg);
    break;
  case P2_CLIENT_WAITING:
    jz_channel_process_server_message_in_P2_client_waiting_state (C, msg);
    break;
  case P3_SERVER_WAITING:
    jz_channel_process_server_message_in_P3_server_waiting_state (C, msg);
    break;
  case D1_FLOW_CONTROL_READY:
    jz_channel_process_server_message_in_D1_flow_control_ready_state (C, msg);
    break;
  case D2_CLIENT_RESET_REQUEST:
    jz_channel_process_server_message_in_D2_client_reset_request_state (C, msg);
    break;
  case D3_SERVER_RESET_INDICATION:
    jz_channel_process_server_message_in_D3_server_reset_indication_state (C, msg);
    break;
  case P5_CALL_COLLISION:
    jz_channel_process_server_message_in_P5_call_collision_state (C, msg);
    break;
  case P6_CLIENT_CLEAR_REQUEST:
    jz_channel_process_server_message_in_P6_client_clear_request_state (C, msg);
    break;
  case P7_SERVER_CLEAR_INDICATION:
    jz_channel_process_server_message_in_P7_server_clear_indication_state (C, msg);
    break;
  }
  jz_channel_dump (C);
}

void jz_channel_process_server_message_in_P1_ready_state (JzChannel *X, JzMsg *msg)
{
  if (msg->id == JZ_MSG_CLEAR_REQUEST)
	jz_channel_do_server_clear_indication (X, msg);

  else if (msg->id == JZ_MSG_CALL_REQUEST)
    jz_channel_do_server_incoming_call (X, msg);
}

void jz_channel_process_server_message_in_P2_client_waiting_state (JzChannel *X, JzMsg *msg)
{
  if (msg->id == JZ_MSG_CLEAR_REQUEST)
	jz_channel_do_server_clear_indication (X, msg);
  
  else if (msg->id == JZ_MSG_CALL_REQUEST)
	jz_channel_do_server_call_collision (X, msg);
  else if (msg->id == JZ_MSG_CALL_ACCEPTED)
    jz_channel_do_server_call_connected (X, msg);
}

void jz_channel_process_server_message_in_P3_server_waiting_state (JzChannel *X, JzMsg *msg)
{
  if (msg->id == JZ_MSG_CLEAR_REQUEST)
	jz_channel_do_server_clear_indication (X, msg);
}

void jz_channel_process_server_message_in_D1_flow_control_ready_state (JzChannel *X, JzMsg *msg)
{
  if (msg->id == JZ_MSG_CLEAR_REQUEST)
	jz_channel_do_server_clear_indication (X, msg);
  
  else if (msg->id == JZ_MSG_RESET_REQUEST)
    jz_channel_do_server_reset_indication (X, msg);
  
  else if (msg->id == JZ_MSG_DATA)
    jz_channel_do_server_data (X, msg);
  else if (msg->id == JZ_MSG_RR)
    jz_channel_do_server_rr (X, msg);
  else if (msg->id == JZ_MSG_RNR)
    jz_channel_do_server_rnr (X, msg);
  else
    {
    }
      
}

void jz_channel_process_server_message_in_D2_client_reset_request_state (JzChannel *X, JzMsg *msg)
{
  if (msg->id == JZ_MSG_CLEAR_REQUEST)
	jz_channel_do_server_clear_indication (X, msg);
  
  else if (msg->id == JZ_MSG_RESET_REQUEST || msg->id == JZ_MSG_RESET_CONFIRMATION)
	jz_channel_do_server_reset_confirmation (X, msg);
}

void jz_channel_process_server_message_in_D3_server_reset_indication_state (JzChannel *X, JzMsg *msg)
{
  if (msg->id == JZ_MSG_CLEAR_REQUEST)
	jz_channel_do_server_clear_indication (X, msg);
  
  else if (msg->id == JZ_MSG_RESET_REQUEST)
	{
	  // IGNORE
	}
}

void jz_channel_process_server_message_in_P5_call_collision_state (JzChannel *X, JzMsg *msg)
{
  if (msg->id == JZ_MSG_CLEAR_REQUEST)
	jz_channel_do_server_clear_indication (X, msg);
  
  else if (msg->id == JZ_MSG_CALL_ACCEPTED)
	jz_channel_do_server_call_connected (X, msg);
  
}

void jz_channel_process_server_message_in_P6_client_clear_request_state (JzChannel *X, JzMsg *msg)
{
  if (msg->id == JZ_MSG_CLEAR_REQUEST || msg->id == JZ_MSG_CLEAR_CONFIRMATION)
	jz_channel_do_server_clear_confirmation (X, msg);
}

void jz_channel_process_server_message_in_P7_server_clear_indication_state (JzChannel *X, JzMsg *msg)
{
  if (msg->id == JZ_MSG_CLEAR_REQUEST)
    {
      // Ignore the message
    }
}

void jz_channel_do_server_clear_indication (JzChannel *X, JzMsg *msg)
{
  X->state = P7_SERVER_CLEAR_INDICATION;
  X->peer = NULL;

  if (X->timer_type != TIMER_CLEAR_INDICATION)
	X->timer_count = 0;
  X->timer_type = TIMER_CLEAR_INDICATION;
  X->timer_expiration_time = (g_get_monotonic_time()
							  + TIMER_CLEAR_INDICATION_DURATION_IN_SECONDS * G_USEC_PER_SEC);

  // Send the clear inidication to the client
  JzMsg *msg2 = jz_msg_new_clear_request (msg->lcn, msg->cause, msg->diagnostic);
  jz_client_send_msg (X->parent, msg2);
  jz_msg_free (msg2);

}

void jz_channel_do_server_incoming_call (JzChannel *X, JzMsg *msg)
{
  // Change state so as to wait for a CALL_ACCEPTED
  X->state = P3_SERVER_WAITING;
  
  // Cache the facilities in this request, so that when we get a reply
  // from the client, we can check if it negotiated facilities fairly
  X->window = msg->window;
  X->throughput = msg->throughput;
  X->packet = msg->packet;

  if (X->timer_type != TIMER_INCOMING_CALL)
	X->timer_count = 0;
  X->timer_type = TIMER_INCOMING_CALL;
  X->timer_expiration_time = (g_get_monotonic_time()
							  + TIMER_INCOMING_CALL_DURATION_IN_SECONDS * G_USEC_PER_SEC);

  // Send the call request to the client
  JzMsg *msg2 = jz_msg_new_call_request (msg->lcn, msg->calling_address, msg->called_address, msg->packet,
                                         msg->window, msg->throughput,
										 g_memdup(msg->data->data, msg->data->len), msg->data->len);
  
  jz_client_send_msg (X->parent, msg2);
  jz_msg_free (msg2);

}

void jz_channel_do_server_call_collision (JzChannel *X, JzMsg *msg)
{
  X->state = P5_CALL_COLLISION;
}


void jz_channel_do_server_call_connected (JzChannel *X, JzMsg *msg)
{
  // The connection is complete, so now we can wait for data
  jz_channel_reset_flow_control (X);
  
  // These are the final negotiated values of the facilities
  X->window = msg->window;
  X->throughput = msg->throughput;
  X->packet = msg->packet;
  
  // Forward the CALL_ACCEPTED back to the client
  
  JzMsg *msg2 = jz_msg_new_call_accepted (msg->lcn, msg->calling_address, msg->called_address, msg->packet,
                                          msg->window, msg->throughput,
										  g_memdup(msg->data->data, msg->data->len), msg->data->len);
  
  jz_client_send_msg (X->parent, msg2);
  jz_msg_free (msg2);
}

void jz_channel_do_server_clear_confirmation (JzChannel *X, JzMsg *msg)
{
  X->state = D1_FLOW_CONTROL_READY;

  // Forward the CLEAR_CONFIRMATION back to the client
  JzMsg *msg2 = jz_msg_new_clear_confirmation (msg->lcn);
  
  jz_client_send_msg (X->parent, msg2);
  jz_msg_free (msg2);
}

void jz_channel_do_server_reset_indication (JzChannel *X, JzMsg *msg)
{
  // The reset request has been received might be a punishment reset
  // with "remote procedure error" as a cause or it might be a "client
  // originated" reset.  In either case, it is forwarded to the client, and the state is
  // changed.

  X->state = D3_SERVER_RESET_INDICATION;
  
  if (X->timer_type != TIMER_RESET_INDICATION)
	X->timer_count = 0;
  X->timer_type = TIMER_RESET_INDICATION;
  X->timer_expiration_time = (g_get_monotonic_time()
							  + TIMER_RESET_INDICATION_DURATION_IN_SECONDS * G_USEC_PER_SEC);

  JzMsg *msg2 = jz_msg_new_reset_request (msg->lcn, msg->cause, msg->diagnostic);
  jz_client_send_msg (X->parent, msg2);
  jz_msg_free (msg2);

  
}

void jz_channel_do_server_reset_confirmation (JzChannel *X, JzMsg *msg)
{
  jz_channel_reset_flow_control (X);
  
  JzMsg *msg2 = jz_msg_new_reset_confirmation (msg->lcn);
  jz_client_send_msg (X->parent, msg2);
  jz_msg_free (msg2);
}

void jz_channel_do_server_rr (JzChannel *X, JzMsg *msg)
{
  g_assert (seq_in_range (msg->pr, X->peer_pr, X->client_ps));

  X->peer_pr = msg->pr;

  JzMsg *msg2 = jz_msg_new_rr (msg->lcn, msg->pr);
  jz_client_send_msg (X->parent, msg2);
  jz_msg_free (msg2);
}

void jz_channel_do_server_rnr (JzChannel *X, JzMsg *msg)
{
  g_assert (seq_in_range (msg->pr, X->peer_pr, X->client_ps));

  X->peer_pr = msg->pr;

  JzMsg *msg2 = jz_msg_new_rnr (msg->lcn, msg->pr);
  jz_client_send_msg  (X->parent, msg2);
  jz_msg_free (msg2);
}

void jz_channel_do_server_data (JzChannel *X, JzMsg *msg)
{
  
  // The data payload is invalid if it is larger than the agreed-upon
  // packet size for this channel.
  g_assert (msg->data->len <= packet_bytes(X->packet));
  g_assert (msg->data->len != 0);
  g_assert (seq_in_range (msg->ps, X->peer_pr, (X->peer_pr + X->window - 1) % SEQ_MAX));
  
  // Forward the packet on to the client
  X->peer_pr = msg->pr;
  X->peer_ps = msg->ps;

  JzMsg *msgX = jz_msg_new_data (msg->lcn, msg->q, msg->pr, msg->ps,
                                 g_memdup(msg->data->data, msg->data->len),
								 msg->data->len);
  jz_client_send_msg (X->parent, msgX);
  jz_msg_free (msgX);
}

////////////////////////////////////////////////////////////////
void
jz_channel_check_timers_wrapper (gpointer key, gpointer value, gpointer user_data)
{
  jz_channel_check_timers ((JzChannel *) value);
}

void jz_channel_check_timers (JzChannel *X)
{
  gint64 now = g_get_monotonic_time ();

  if (X->timer_type == TIMER_NONE || X->timer_expiration_time > now)
	return;

  X->timer_count ++;

  printf("Timer %d, expiration %llu, count %d\n", X->timer_type, X->timer_expiration_time, X->timer_count);
  switch (X->timer_type)
	{
	case TIMER_NONE:
	  g_assert_not_reached ();
	  break;
	case TIMER_INCOMING_CALL:
	  jz_channel_do_incoming_call_expiration (X);
	  break;
	case TIMER_RESET_INDICATION:
	  jz_channel_do_reset_indication_expiration (X);
	  break;
	case TIMER_CLEAR_INDICATION:
	  jz_channel_do_clear_indication_expiration (X);
	  break;
	default:
	  g_assert_not_reached ();
	}
}

void jz_channel_timer_reset (JzChannel *X)
{
  X->timer_type = TIMER_NONE;
  X->timer_expiration_time = 0;
  X->timer_count = 0;
}

void jz_channel_do_incoming_call_expiration (JzChannel *X)
{
  g_assert (X->timer_count == 1);

  jz_channel_timer_reset (X);
  jz_channel_do_punishment_clear_request (X, c_clear__local_procedure_error, d_time_expired_incoming_call);
}

void jz_channel_do_reset_indication_expiration (JzChannel *X)
{
  g_assert (X->timer_count == 1 || X->timer_count == 2);

  switch (X->timer_count)
	{
	case 1:
	  jz_channel_do_punishment_reset_request (X, c_reset__local_procedure_error, d_time_expired_reset_request);
	  break;
	case 2:
	  jz_channel_do_punishment_clear_request (X, c_clear__local_procedure_error, d_time_expired_reset_request);
	  break;
	default:
	  g_assert_not_reached ();
	  break;
	}
}

void jz_channel_do_clear_indication_expiration (JzChannel *X)
{
  g_assert (X->timer_count == 1 || X->timer_count == 2);
  JzMsg *msg;

  switch (X->timer_count)
	{
	case 1:
	  jz_channel_do_punishment_clear_request (X, c_clear__local_procedure_error, d_time_expired_clear_request);
	  break;
	case 2:
	  if (X->peer)
		{
		  X->peer->state = P1_READY;
		  X->peer->peer = NULL;
		}
	  X->state = P1_READY;
	  X->peer = NULL;
	  jz_channel_timer_reset (X);
	  msg = jz_msg_new_diagnostic (d_time_expired_clear_request, JZ_MSG_VERSION,
								   JZ_MSG_CLEAR_REQUEST, X->lcn);
	  jz_client_send_msg (X->parent, msg);
	  jz_msg_free (msg);
	  break;
	default:
	  g_assert_not_reached ();
	  break;
	}
}
