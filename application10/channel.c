#include "channel.h"

void jz_channel_process_message_in_P1_ready_state (JzChannel *X, JzMsg *msg);
void jz_channel_process_message_in_P2_client_waiting_state (JzChannel *X, JzMsg *msg);
void jz_channel_process_message_in_P3_server_waiting_state (JzChannel *X, JzMsg *msg);
void jz_channel_process_message_in_D1_flow_control_ready_state (JzChannel *X, JzMsg *msg);
void jz_channel_process_message_in_D2_client_reset_request_state (JzChannel *X, JzMsg *msg);
void jz_channel_process_message_in_D3_server_reset_request_state (JzChannel *X, JzMsg *msg);

void jz_channel_process_server_message (JzChannel *C, JzMsg *msg);
void jz_channel_process_server_message_in_P1_ready_state (JzChannel *X, JzMsg *msg);
void jz_channel_process_server_message_in_P2_client_waiting_state (JzChannel *X, JzMsg *msg);


void jz_channel_do_client_call_request (JzChannel *X, JzMsg *msg);
void jz_channel_do_client_call_accepted (JzChannel *X, JzMsg *msg);
void jz_channel_do_client_call_collision (JzChannel *X, JzMsg *msg);
void jz_channel_do_client_clear_request (JzChannel *X, JzMsg *msg);

void jz_channel_do_server_clear_request (JzChannel *X, cause_t cause, diag_t diagnostic);


void jz_channel_process_message_in_D3_server_reset_request_state (JzChannel *X, JzMsg *msg);


JzChannel *jz_channel_new (JzClient *client)
{
  JzChannel *C = g_new0 (JzChannel, 1);
  C->parent = client;
  C->peer = NULL;
  C->state = P1_READY;
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

void jz_channel_process_message (JzChannel *C, JzMsg *msg)
{
  switch (C->state) {
  case P1_READY:
    jz_channel_process_message_in_P1_ready_state (C, msg);
    break;
  case P2_CLIENT_WAITING:
    jz_channel_process_message_in_P2_client_waiting_state (C, msg);
    break;
  case P3_SERVER_WAITING:
    jz_channel_process_message_in_P3_server_waiting_state (C, msg);
    break;
  case P5_CALL_COLLISION:
    // jz_channel_process_message_in_P5_call_collision_state (C, msg);
    break;
  case P6_CLIENT_CLEAR_REQUEST:
    // jz_channel_process_message_in_P6_client_clear_request_state (C, msg);
    break;
  case P7_SERVER_CLEAR_REQUEST:
    // jz_channel_process_message_in_P7_server_clear_request_state (C, msg);
    break;

  case D1_FLOW_CONTROL_READY:
    jz_channel_process_message_in_D1_flow_control_ready_state (C, msg);
    break;
  case D2_CLIENT_RESET_REQUEST:
    jz_channel_process_message_in_D2_client_reset_request_state (C, msg);
    break;
  case D3_SERVER_RESET_REQUEST:
    jz_channel_process_message_in_D3_server_reset_request_state (C, msg);
    break;
  }  
}

void jz_channel_process_message_in_P1_ready_state (JzChannel *X, JzMsg *msg)
{
  if (msg->id == JZ_MSG_CALL_REQUEST)
    jz_channel_do_client_call_request (X, msg);
  else if (msg->id == JZ_MSG_CLEAR_REQUEST)
    jz_channel_do_client_clear_request (X, msg);
  else
    jz_channel_do_server_clear_request (X, c_local_procedure_error, d_invalid_message_for_state_ready);
}

void jz_channel_process_message_in_P2_client_waiting_state (JzChannel *X, JzMsg *msg)
{
  if (msg->id == JZ_MSG_CLEAR_REQUEST)
    jz_channel_do_client_clear_request (X, msg);
  else
    jz_channel_do_server_clear_request (X, c_local_procedure_error, d_invalid_message_for_state_client_waiting);
}

void jz_channel_process_message_in_P3_server_waiting_state (JzChannel *X, JzMsg *msg)
{
  if (msg->id == JZ_MSG_CALL_REQUEST)
    jz_channel_do_client_call_collision (X, msg);
  else if (msg->id == JZ_MSG_CALL_ACCEPTED)
    jz_channel_do_client_call_accepted (X, msg);
  else if (msg->id == JZ_MSG_CLEAR_REQUEST)
    jz_channel_do_client_clear_request (X, msg);
  else
    jz_channel_do_server_clear_request (X, c_local_procedure_error, d_invalid_message_for_state_server_waiting);
}

void jz_channel_process_message_in_D1_flow_control_ready_state (JzChannel *X, JzMsg *msg)
{
  if (msg->id == JZ_MSG_CLEAR_REQUEST)
    jz_channel_do_client_clear_request (X, msg);

  if (msg->id == JZ_MSG_RESET_REQUEST)
    //jz_channel_do_client_reset_request (X, msg)
    ;
  else if (msg->id == JZ_MSG_DATA)
    //jz_channel_do_client_data (X, msg)
    ;
  else if (msg->id == JZ_MSG_RR)
    //jz_channel_do_client_rr_request (X, msg)
    ;
  else if (msg->id == JZ_MSG_RNR)
    //jz_channel_do_client_rnr_request (X, msg)
    ;

  else
    //jz_channel_do_server_reset_request (X, c_local_procedure_error, d_invalid_message_for_state_flow_control_ready)
    ;

}

void jz_channel_process_message_in_D2_client_reset_request_state (JzChannel *X, JzMsg *msg)
{
  if (msg->id == JZ_MSG_CLEAR_REQUEST)
    jz_channel_do_client_clear_request (X, msg);

  if (msg->id == JZ_MSG_RESET_REQUEST)
    // ignore
    ;
  else
    //jz_channel_do_server_reset_request (X, c_local_procedure_error,
    //                                    d_invalid_message_for_state_client_reset_request)
    ;

}

void jz_channel_process_message_in_D3_server_reset_request_state (JzChannel *X, JzMsg *msg)
{
  if (msg->id == JZ_MSG_CLEAR_REQUEST)
    jz_channel_do_client_clear_request (X, msg);

  if (msg->id == JZ_MSG_RESET_REQUEST || msg->id == JZ_MSG_RESET_CONFIRMATION)
    //jz_channel_do_reset_confirmation (X, msg)
    ;
  else
    // ignore
    ;

}

void jz_channel_do_client_call_request (JzChannel *X, JzMsg *msg)
{
  cause_t cause;
  diag_t diagnostic;

  // UNIMPLEMENTED: Here X.25 would check if a particular LCN is a one-way incoming channel
  //  but we haven't chosen different behaviors for different LCNs yet.
  // Here we check if the client has outgoing calls barred.  N.B. This
  // is different from the unimplemented "one way logical channel"
  // above by being a function of all the channels in the client.
  if (X->parent->iodir == io_outgoing_calls_barred || X->parent->iodir == io_calls_barred)
    {
      cause = c_local_procedure_error;
      diagnostic = d_outgoing_calls_barred;
      goto fail;
    }
  // Here X.25 would re-check the basic message validity, but, we did than when the
  // message was received.
  // X.25: check if address contains non-BCD digit
  // X.25: check in calling address is invalid
  // X.25: check if called address is invalid
  // X.25: check facility codes and values

  // Here we check if there is a peer with the given name
  JzClient *peer;
  if ((peer = jz_client_list_find_client (msg->called_address)) == NULL)
    {
      cause = c_unknown_address;
      diagnostic = d_invalid_called_address;
      goto fail;
    }

  // Here we check if the called client accepts incoming calls
  if (peer->iodir == io_incoming_calls_barred || peer->iodir == io_calls_barred)
    {
      cause = c_access_barred;
      diagnostic = d_input_barred;
      goto fail;
    }

  // Here we check if the called client has a free channel
  JzChannel *Y;
  if ((Y = jz_client_reserve_low_channel (peer)) == 0)
    {
      cause = c_number_busy;
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
                                         msg->data);
  jz_channel_process_server_message (Y, msg);
  jz_msg_free (msg2);

  // Start waiting for a reply
  X->state = P2_CLIENT_WAITING;

  return;

 fail:
  jz_channel_do_server_clear_request (X, cause, diagnostic);
}

void jz_channel_do_client_call_accepted (JzChannel *X, JzMsg *msg)
{
  // FIXME: Check that the client hasn't cheated in the facility negotiation
  X->window = msg->window;
  X->throughput = msg->throughput;
  X->packet = msg->packet;
  
  // Then, forward it on to the other side of the connection
  g_assert (X->peer);
  JzMsg *msgY = jz_msg_new_call_accepted (X->peer->lcn, msg->calling_address, msg->called_address,
                                          msg->packet, msg->window, msg->throughput, msg->data);
  jz_channel_process_server_message (X->peer, msgY);
  X->state = D1_FLOW_CONTROL_READY;
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
                                         msg->packet, msg->window, msg->throughput, msg->data);
  jz_channel_process_server_message (X->peer, msgY);
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
      JzMsg *msg2 = jz_msg_new_clear_request (X->peer->lcn, c_client_originated, d_unspecified);
      jz_channel_process_server_message (X->peer, msg2);
      jz_msg_free (msg2);
    }

  // Start waiting for a reply
  X->state = P6_CLIENT_CLEAR_REQUEST;
}

void jz_channel_do_server_clear_request (JzChannel *X, cause_t cause, diag_t diagnostic)
{
  JzMsg *msgX = jz_msg_new_clear_request (X->lcn, cause, diagnostic);
  jz_client_send_msg (X->parent, msgX);
  jz_msg_free (msgX);

  if (X->peer)
    {
      if (cause == c_local_procedure_error)
        cause = c_remote_procedure_error;
      JzMsg *msgY = jz_msg_new_clear_request (X->peer->lcn, cause,
                                             diagnostic);
      jz_channel_process_server_message (X->peer, msgY);
    }
  X->state = P7_SERVER_CLEAR_REQUEST;
}

// X has requested a RESTART, so for each channel on X, it is set to the READY state,
// disconnected from the channel, and the peer is sent a server clear request
void jz_channel_do_client_restart_request (JzChannel *X, diag_t diagnostic)
{
  JzChannel *Y = X->peer;

  X->peer->peer = NULL;
  X->peer = NULL;

  if (Y)
    {
      JzMsg *msgY = jz_msg_new_clear_request (Y->lcn, c_client_originated, diagnostic);
      jz_channel_process_server_message (Y, msgY);
      jz_msg_free (msgY);
    }

  X->state = P1_READY;
}

// X has goofed, and the server is demanding a RESTART, so for each
// channel on X, it is set to the READY state, disconnected from the
// channel, and the peer is sent a server clear request
void jz_channel_do_server_restart_request (JzChannel *X, diag_t diagnostic)
{
  JzChannel *Y = X->peer;

  X->peer->peer = NULL;
  X->peer = NULL;

  if (Y)
    {
      JzMsg *msgY = jz_msg_new_clear_request (Y->lcn, c_remote_procedure_error,
                                             diagnostic);
      jz_channel_process_server_message (Y, msgY);
      jz_msg_free (msgY);
    }

  X->state = P1_READY;
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
    // jz_channel_process_server_message_in_P3_server_waiting_state (C, msg)
    ;
    break;
  case P5_CALL_COLLISION:
    // jz_channel_process_server_message_in_P5_call_collision_state (C, msg)
    ;
    break;
  case P6_CLIENT_CLEAR_REQUEST:
    // jz_channel_process_server_message_in_P6_client_clear_request_state (C, msg)
    ;
    break;
  case P7_SERVER_CLEAR_REQUEST:
    // jz_channel_process_server_message_in_P7_server_clear_request_state (C, msg)
    ;
    break;
  }  
}

void jz_channel_process_server_message_in_P1_ready_state (JzChannel *X, JzMsg *msg)
{
  if (msg->id == JZ_MSG_CALL_REQUEST)
    {
      // internal messages are assumed to be valid.

      // Change state so as to wait for a CALL_ACCEPTED
      X->state = P3_SERVER_WAITING;

      // Cache the facilities in this request, so that when we get a reply
      // from the client, we can check if it negotiated facilities fairly
      X->window = msg->window;
      X->throughput = msg->throughput;
      X->packet = msg->packet;

      // Send the call request to the client
      JzMsg *msg2 = jz_msg_new_call_request (msg->lcn, msg->calling_address, msg->called_address, msg->packet,
                                             msg->window, msg->throughput, msg->data);
      
      jz_client_send_msg (X->parent, msg2);
      jz_msg_free (msg2);
    }
}

void jz_channel_process_server_message_in_P2_client_waiting_state (JzChannel *X, JzMsg *msg)
{
  if (msg->id == JZ_MSG_CALL_ACCEPTED)
    {
      // internal messages are assumed to be valid.

      // The connection is complete, so now we can wait for data
      X->state = D1_FLOW_CONTROL_READY;

      // These are the final negotiated values of the facilities
      X->window = msg->window;
      X->throughput = msg->throughput;
      X->packet = msg->packet;

      // Forward the CALL_ACCEPTED back to the client

      JzMsg *msg2 = jz_msg_new_call_accepted (msg->lcn, msg->calling_address, msg->called_address, msg->packet,
                                              msg->window, msg->throughput, msg->data);
      
      jz_client_send_msg (X->parent, msg2);
      jz_msg_free (msg2);
    }
}
