#include <gio/gio.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <string.h>
#include "_gsocketconnection.h"
#include "client.h"
#include "msg.h"
#include "cause.h"
#include "diag.h"
#include "channel.h"

GList *client_list;

static const char client_state_names[4][15] = {
  [C1_UNITIALIZED] = "UNINITIALIZED",
  [C2_CLIENT_RESTART_REQUEST] = "CLIENT_RESTART",
  [C3_SERVER_RESTART_REQUEST] = "SERVER_RESTART",
  [C4_INITIALIZED] = "INITIALIZED",
};

static const char client_short_state_names[4][15] = {
  [C1_UNITIALIZED] = "UNINIT",
  [C2_CLIENT_RESTART_REQUEST] = "C_RESTART",
  [C3_SERVER_RESTART_REQUEST] = "S_RESTART",
  [C4_INITIALIZED] = "INIT",
};


static const char *client_state_name(client_state_t C)
{
    return client_state_names[C];
}

static const char *client_short_state_name(client_state_t C)
{
  return client_short_state_names[C];
}


static gboolean _jz_client_parse_recv_buffer_into_messages (JzClient *C, diag_t *diag);
static gboolean _jz_client_read_into_recv_buffer (JzClient *C, GPollableInputStream *pollable);
static void
_jz_client_process_messages (JzClient *C);
static void
_jz_client_process_fatally_invalid_message (JzClient *C);
void
_jz_client_process_message_in_C1_uninitialized_state (JzClient *C, JzMsg *msg);
void
_jz_client_process_message_in_C2_client_restart_request_state (JzClient *C, JzMsg *msg);
void
_jz_client_process_message_in_C3_server_restart_request_state (JzClient *C, JzMsg *msg);
void
_jz_client_process_message_in_C4_initialized_state (JzClient *C, JzMsg *msg);
void
jz_client_do_diagnostic (JzClient *C, guint8 cause, guint8 diagnostic);
void
jz_client_do_connect (JzClient *C, JzMsg *msg);
void
jz_client_do_disconnect (JzClient *C);
void
jz_client_do_restart (JzClient *C, cause_t cause, diag_t diagnostic);
void
jz_client_do_server_restart (JzClient *C, cause_t cause, diag_t diagnostic);
void
jz_client_do_restart_confirmation (JzClient *C);
const gchar *
jz_client_diagnostic_string (JzClient *C);


// Creates a new JzClient, freshly allocated
JzClient *
jz_client_new (void)
{
  JzClient *C;
  C = g_new0 (JzClient, 1);

  // Here we'd initialize all the members that are not zero or NULL.
  C->iodir = io_calls_barred;
  C->incoming = g_queue_new ();
  C->channels = g_hash_table_new (g_direct_hash, g_direct_equal);

  return C;
}

// Frees the memory allocated for the JzClient. Only call this
// function if queue was created with jz_client_new().
void
jz_client_free (JzClient *C)
{
  // Free all the private members.
  C->has_connection = FALSE;
  if (C->connection != NULL)
    g_io_stream_close (G_IO_STREAM (C->connection), NULL, NULL);
  C->connection = NULL;

  g_free (C->inet_address_str);
  C->port = 0;

  g_source_destroy (C->input_stream_source);
  C->input_stream_source = NULL;

  C->state = C1_UNITIALIZED;
  g_free (C->address);
  C->address = NULL;

  C->iodir = io_calls_barred;

  memset (C->recv_buffer, sizeof(C->recv_buffer), 0);
  C->recv_buffer_size = 0;

  g_queue_free_full (C->incoming, jz_msg_free);
  C->incoming = NULL;

  g_hash_table_unref (C->channels);
  C->channels = NULL;
  // Free the structure itself
  g_free (C);
}

////////////////////////////////////////////////////////////////
// INPUT PROCESSING FROM CLIENT
////////////////////////////////////////////////////////////////

// This type maps to type GPollableSourceFunc.  It is called when a socket connection
// reports that there is input data available.
static gboolean
_jz_client_input_available_callback (GObject *pollable_stream, JzClient *C)
{
  GPollableInputStream *pollable_input_stream = (GPollableInputStream *) pollable_stream;

    if (_jz_client_read_into_recv_buffer (C, pollable_input_stream))
    {
        diag_t diag;
      if (_jz_client_parse_recv_buffer_into_messages (C, &diag))
        _jz_client_process_messages (C);
      else
        _jz_client_process_fatally_invalid_message (C);
    }
  else
    {
      const gchar *str = jz_client_diagnostic_string (C);
      g_message ("%s: REMOVED", str);

      // Remove this source from further processing
      return FALSE;
    }
  return G_SOURCE_CONTINUE;

}

static gboolean
_jz_client_read_into_recv_buffer (JzClient *C, GPollableInputStream *pollable)
{
  gssize res;
  GError *error;
  const gchar *client_name = jz_client_diagnostic_string (C);

  g_assert (C->recv_buffer_size < sizeof (C->recv_buffer));

  error = NULL;
  res = g_pollable_input_stream_read_nonblocking (pollable, &C->recv_buffer[C->recv_buffer_size],
                                                  sizeof (C->recv_buffer) - C->recv_buffer_size,
                                                  NULL, &error);

  if (res > 0)
    {
      g_message ("%s: received %d bytes", client_name, res);
      C->recv_buffer_size += res;
    }
  else if (res < 0 && g_error_matches (error, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK))
    {
      g_error_free (error);
      res = 0;
    }
  else if (res <= 0)
    {
      if (error)
        g_message ("%s: %s", client_name, error ? error->message : "eof");

      // Since the client has disappeared, returning G_SOURCE_REMOVE removes
      // this socket from being queried.
      return G_SOURCE_REMOVE;
    }

  // Returning G_SOURCE_CONTINUE here means that we'll keep polling this socket.
  return G_SOURCE_CONTINUE;
}

static gboolean
_jz_client_parse_recv_buffer_into_messages (JzClient *C, diag_t *diag)
{
  guint8 *p = NULL, *end = NULL;
  JzMsg *msg = NULL;
  gboolean ret = TRUE;

  p = C->recv_buffer;
  end = p + C->recv_buffer_size;

  while ((p < end) && (ret == TRUE))
    {
      // The smallest possible message is 12 bytes, which is just an envelope
      if (end - p < JZ_MSG_ENVELOPE_SIZE)
        {
          // We haven't even received enough data for a message envelope.
          // Do nothing, for now
          break;
        }
      // A message is supposed to start with '~SVC'
      else if (!jz_buffer_begins_with_signature (p, end - p))
        {
          // The message envelope didn't start with the right string.
          // We're hopelessly confused now and can't recover.
          ret = FALSE;
          break;
        }
      else if (!jz_buffer_contains_a_message (p, end - p))
        {
          // We haven't received enough data yet for this message.  Do
          // nothing, for now.
          break;
        }
      else if (!jz_buffer_msg_envelope_is_valid (p, end - p))
        {
          // Bad envelope usually means bad checksum.
          // FIXME: extract version, ID, and lcn for the following call
          JzMsg *reply = jz_msg_new_diagnostic (d_unidentifiable_packet, 0, 0, 0);
          jz_client_send_msg (C, reply);
          jz_msg_free (reply);
          
          // Don't unpack this message.  Just advance to the next message.
          p += jz_buffer_msg_binary_size (p, end - p);
          
        }
      else if (jz_buffer_msg_body_length (p, end - p) < JZ_MSG_PAYLOAD_HEADER_SIZE)
        {
          // Check that the message body at least contains a complete header
          // FIXME: extract version, ID, and LCN for the following call
          JzMsg *reply = jz_msg_new_diagnostic (d_packet_too_short, 0, 0, 0);
          jz_client_send_msg (C, reply);
          jz_msg_free (reply);
          
          // Don't unpack this message.  Just advance to the next message.
          p += jz_buffer_msg_binary_size (p, end - p);
        }
      else if (jz_buffer_msg_lcn (p, end - p) > JZ_MSG_MAX_LCN)
        {
          // This LCN is out of range for this client
          // FIXME: extract version, ID, and LCN for the following call
          JzMsg *reply = jz_msg_new_diagnostic (d_packet_on_unassigned_logical_channel, 0, 0, 0);
          jz_client_send_msg (C, reply);
          jz_msg_free (reply);

          // Don't unpack this message.  Just advance to the next
          // message.
          p += jz_buffer_msg_binary_size (p, end - p);
        }
      else
        {
          // If we get this far, it is safe to attempt to unpack the
          // message payload
          msg = jz_msg_new_from_data (p, end - p);
          if (!msg->valid)
            {
              // The message payload appears to be too short or too
              // long.
              JzMsg *reply = jz_msg_new_diagnostic (msg->invalidity, msg->version, msg->id, msg->lcn);
              jz_client_send_msg (C, reply);
              jz_msg_free (reply);

              // Advance to the next message
              p += jz_msg_binary_size (msg);

              // Dump the message
              jz_msg_free (msg);
            }
          else
            {
              // There is a complete message envelope and payload, so
              // we can finally queue it.
              g_queue_push_tail (C->incoming, msg);
              g_message ("%s: queued %s",
                         jz_client_diagnostic_string (C), id_name (msg->id));

              // Advance to the next message
              p += jz_msg_binary_size (msg);
            }
        }

    }

  if (p < end)
    memmove (C->recv_buffer, p, end - p);
  C->recv_buffer_size = end - p;
  return ret;
}

// The client should have unpacked messages in its message storage
// when this function is called.
static void
_jz_client_process_messages (JzClient *C)
{
  while (!g_queue_is_empty (C->incoming))
    {
      JzMsg *msg = g_queue_pop_head (C->incoming);
      g_message ("%s: processing %s",
                 jz_client_diagnostic_string (C),
                 id_name (msg->id));
      diag_t err = jz_msg_validate (msg);
      if (err == d_ok)
        {
          if (C->state == C1_UNITIALIZED)
            _jz_client_process_message_in_C1_uninitialized_state (C, msg);
          else if (C->state == C2_CLIENT_RESTART_REQUEST)
            _jz_client_process_message_in_C2_client_restart_request_state (C, msg);
          else if (C->state == C3_SERVER_RESTART_REQUEST)
            _jz_client_process_message_in_C3_server_restart_request_state (C, msg);
          else if (C->state == C4_INITIALIZED)
            _jz_client_process_message_in_C4_initialized_state (C, msg);
        }
      else
        g_message ("Invalid message type %s in queue", id_name (msg->id));
      jz_msg_free (msg);
    }
}

void
_jz_client_process_message_in_C1_uninitialized_state (JzClient *C, JzMsg *msg)
{
  // The only valid message for an unitialized client is CONNECT
  if (msg->id == JZ_MSG_CONNECT) {
    // Check if this calling address is in use.
    gboolean name_in_use = FALSE;
    for (GList *l = client_list; l != NULL; l = l->next)
      {
        JzClient *C2 = (JzClient *) (l->data);
        if (C2->address && g_str_equal (C2->address, msg->calling_address))
          {
            name_in_use = TRUE;
            break;
          }
      }
    if (name_in_use)
      jz_client_do_diagnostic (C, c_address_in_use, d_address_in_use);
    else
      jz_client_do_connect (C, msg);
  }
  else
    // Ignore the message
    ;
}

void
_jz_client_process_message_in_C2_client_restart_request_state (JzClient *C, JzMsg *msg)
{
  if (msg->id == JZ_MSG_RESTART_REQUEST)
    // Duplicate restart.  Ignore it.
    ;
  else if (msg->id == JZ_MSG_RESTART_CONFIRMATION || jz_msg_is_for_channel (msg))
    {
      jz_client_do_server_restart (C, c_local_procedure_error, d_invalid_message_for_state_client_restart_request);
    }
  else
    jz_client_do_diagnostic (C, c_local_procedure_error, d_invalid_message_for_state_client_restart_request);
}

void
_jz_client_process_message_in_C3_server_restart_request_state (JzClient *C, JzMsg *msg)
{
  if (msg->id == JZ_MSG_RESTART_REQUEST || msg->id == JZ_MSG_RESTART_CONFIRMATION)
    jz_client_do_restart_confirmation (C);
  else if (jz_msg_is_for_channel (msg))
    {
      // discard silently
    }
  else
    jz_client_do_diagnostic (C, c_local_procedure_error, d_invalid_message_for_state_server_restart_request);
}

void
_jz_client_process_message_in_C4_initialized_state (JzClient *C, JzMsg *msg)
{
  if (msg->id == JZ_MSG_RESTART_REQUEST)
    jz_client_do_restart (C, msg->cause, msg->diagnostic);
  else if (msg->id == JZ_MSG_RESTART_CONFIRMATION)
      jz_client_do_server_restart (C, c_local_procedure_error, d_invalid_message_for_state_initialized);
  else if (jz_msg_is_for_channel (msg))
    {
      // Create a logical channel, if necessary
      if (!g_hash_table_contains (C->channels, GINT_TO_POINTER(msg->lcn)))
        g_hash_table_insert (C->channels, GINT_TO_POINTER (msg->lcn), jz_channel_new (C));

      // Punt processing to that logical channel;
      jz_channel_process_message (g_hash_table_lookup (C->channels, GINT_TO_POINTER (msg->lcn)), msg);
    }
  else
    jz_client_do_diagnostic (C, c_local_procedure_error, d_invalid_message_for_state_initialized);
}

void
jz_client_do_connect (JzClient *C, JzMsg *msg)
{
  // The server connects the client
  JzMsg *msgC = jz_msg_new_connect_indication ();
  jz_client_send_msg (C, msgC);
  jz_msg_free (msgC);

  g_message ("%s: registering as '%s'", jz_client_diagnostic_string (C),
             msg->calling_address);

  C->state = C4_INITIALIZED;
  C->address = g_strdup (msg->calling_address);
  C->iodir = msg->iodir;
}

void
jz_client_do_diagnostic (JzClient *C, guint8 cause, guint8 diagnostic)
{
  JzMsg *msgC = jz_msg_new_diagnostic (cause, diagnostic);
  jz_client_send_msg (C, msgC);
  jz_msg_free (msgC);
}

gboolean _restart_iterator_client (gpointer key, gpointer value, gpointer user_data)
{
  JzChannel *channel = (JzChannel *) value;
  diag_t diagnostic = (GPOINTER_TO_INT (user_data));
  jz_channel_do_client_restart_request (channel, diagnostic);

  // returning TRUE eliminates this channel
  return TRUE;
}

// The client wants to RESTART all of its channels
void
jz_client_do_restart (JzClient *C, cause_t cause, diag_t diagnostic)
{
  if (cause != c_client_originated)
    jz_client_do_server_restart (C, c_local_procedure_error, d_improper_cause_code_from_client);
  else
    {
      C->state = C2_CLIENT_RESTART_REQUEST;

      // Send a CLEAR_INDICATION to the peer of every channel to which
      // we are connected with the cause "remote procedure error" and
      // the same DIAGNOSTIC.
      g_hash_table_foreach_remove (C->channels,
                                   _restart_iterator_client,
                                   GINT_TO_POINTER (diagnostic));

      JzMsg *msgC = jz_msg_new_restart_confirmation ();
      jz_client_send_msg (C, msgC);
      jz_msg_free (msgC);
      C->state = C4_INITIALIZED;
    }
}

gboolean _restart_iterator_server (gpointer key, gpointer value, gpointer user_data)
{
  JzChannel *channel = (JzChannel *) value;
  diag_t diagnostic = (GPOINTER_TO_INT (user_data));
  jz_channel_do_server_restart_request (channel, diagnostic);

  // returning TRUE eliminates this channel
  return TRUE;
}

// The server is punishing the client by restarting all of its channels, usually because
// the client has sent malformed messages from which the server can't recover.
void
jz_client_do_server_restart (JzClient *C, cause_t cause, diag_t diagnostic)
{
  JzMsg *msg2 = jz_msg_new_restart_request (cause, diagnostic);
  jz_client_send_msg (C, msg2);
  jz_msg_free (msg2);

  // Send a CLEAR_INDICATION to the peer of every channel to which we
  // are connected with the cause "remote procedure error" and the
  // same DIAGNOSTIC.
  g_hash_table_foreach_remove (C->channels,
                               _restart_iterator_server,
                               GINT_TO_POINTER(diagnostic));

  // Set the state to "server restart indication" where we'll await
  // the client's reply.
  C->state = C3_SERVER_RESTART_REQUEST;
}
void
jz_client_do_restart_confirmation (JzClient *C)
{
  C->state = C4_INITIALIZED;
}



static void
_jz_client_process_fatally_invalid_message (JzClient *C)
{
  // If the client has sent a malformed message, we can't recover
  // because this isn't a packet-based protocol.  We don't know where
  // the packet begins.  In the future, once could scan data for the
  // initializer "~SVC", but, for now we'll fail with great prejudice.

  // Clear out the receive buffer
  C->recv_buffer_size = 0;

  // And restart everything
  jz_client_do_server_restart (C, c_local_procedure_error, d_unidentifiable_packet);
}

////////////////////////////////////////////////////////////////

void
jz_client_send_msg (JzClient *C, JzMsg *M)
{
  GSocketConnection *sock_con = C->connection;
  GOutputStream *out = g_io_stream_get_output_stream (G_IO_STREAM (sock_con));
  gssize bytes_written;
  GByteArray *b_array = jz_msg_to_byte_array (M);
  bytes_written = g_output_stream_write (out,
                                         b_array->data,
                                         b_array->len,
                                         NULL,
                                         NULL);
  gchar *intro;
  const gchar *addr = jz_client_diagnostic_string (C);
  if (bytes_written >= 0)
    intro = "Sent message";
  else
    intro = "Failed to send message";

  if (M->id == JZ_MSG_DIAGNOSTIC)
    g_message ("%s: %s %s %s", addr, intro, id_name(M->id), diag_name (M->diagnostic));
  else
    g_message ("%s: %s %s", addr, intro, id_name(M->id));
}



// Attaches a socket connection to a JzClient so as to be managed as
// part of a connection to a client.
void jz_client_take_socket_connection (JzClient *C, GSocketConnection *S)
{
  g_assert (C != NULL);
  g_assert (S != NULL);
  // Can't reuse these client connections.
  g_assert (C->has_connection == FALSE);

  C->has_connection = TRUE;
  C->connection = S;
  g_object_ref (C->connection);
  C->inet_address_str = _g_socket_connection_get_remote_address_str (S);
  C->port = _g_socket_connection_get_remote_port (S);
  C->input_stream_source = _g_socket_connection_input_stream_create_source (S);

  // Attach this event source to trigger in the main loop, and add a
  // callback function to call on that trigger.
  g_source_attach (C->input_stream_source, NULL);
  g_source_set_callback (C->input_stream_source,
                         _jz_client_input_available_callback, C, NULL);

  C->state = C1_UNITIALIZED;
  // C->name is stil NULL
  // recv_buffer is still empty
  // incoming list is still empty
}

JzClient *jz_client_list_find_client (gchar *address)
{
  for (GList *l = client_list; l != NULL; l = l->next)
    {
      JzClient *C = (JzClient *) (l->data);
      if (C->address && g_str_equal (C->address, address))
        {
          return C;
        }
    }
  return NULL;
}

JzChannel *jz_client_reserve_low_channel (JzClient *C)
{
  guint16 lcn = 1;
  JzChannel *ch = NULL;
  gboolean key_is_new;

  while (g_hash_table_contains (C->channels, GINT_TO_POINTER (lcn))
         && lcn < JZ_MSG_MAX_LCN)
    lcn ++;
  if (lcn < JZ_MSG_MAX_LCN)
    {
      ch = jz_channel_new (C);
      key_is_new = g_hash_table_insert (C->channels, GINT_TO_POINTER (lcn), ch);
      g_assert (key_is_new == TRUE);
    }

  return ch;
}

char diag_string_buffer[80];
const gchar *
jz_client_diagnostic_string (JzClient *C)
{
  gchar *addr, *output;
  if (C->address)
    addr = g_strdup_printf ("%s", C->address);
  else
    {
      gchar *temp = g_strdup_printf ("%s:%d", C->inet_address_str, C->port);
      addr = g_strdup_printf("%s", temp);
      g_free (temp);
    }

 g_sprintf(diag_string_buffer,
           "%-16s %-9s %2s %d %d",
           addr,
           client_short_state_name(C->state),
           iodir_short_name(C->iodir),
           C->incoming->length,
           g_hash_table_size(C->channels));
  g_free (addr);
  return diag_string_buffer;
}

#if 0
guint32
s_client_hash(gchar *str, uint16 num)
{
  guint32 val = 0;
  int i;
  for (i = 0; i < strlen(str); i ++)
    val += val * 101 + (unsigned) str[i];
  val += val * 101 + (unsigned)((char *)&num)[0];
  val += val * 101 + (unsigned)((char *)&num)[1];
  return val;
}


// For a client, take the raw data in the receive buffer
// and chunck it into separate messages packets.  Place
// these message packets into the client's list of
// unprocessed messages.
static void
parse_all_input (client_t *client)
{
  guint8 *p, *end;
  guint32 size, magic;
  joza_msg_t msg;

  p = server->recv_buffer;
  end = p + server->recv_buffer_size;

  while (p + JOZA_MSG_HEADER_SIZE <= end)
    {
      joza_msg_t *msg;
      size = joza_msg_extract (p, end - p, &msg);

      if (size > 0)
        {
          // There is a complete message in the data
          client->incoming = g_list_append (client->incoming, msg);
          p += size;
        }
      else if (size == 0)
        {
          // There isn't a complete message in the data
        }
      else if (size < 0)
        {
          // The data doesn't validate into messages
          // Trigger a server-requested restart
        }
    }

  if (p < end)
    memmove (client->recv_buffer, p, end - p);
  client->recv_buffer_size = end - p;
}

// For a given socket connection with a client, read any available
// input data from the socket and store it into a client-specific
// receive buffer.
static void
read_some_input_nonblocking (client_t *client)
{
  GInputStream *in;
  GPollableInputStream *pollable;
  gssize res;
  GError *error;

  in = g_io_stream_get_input_stream (G_IO_STREAM (client->connection));
  pollable = G_POLLABLE_INPUT_STREAM (in);

  g_assert (client->recv_buffer_size < sizeof (client->recv_buffer));
  error = NULL;
  res = g_pollable_input_stream_read_nonblocking (pollable, &client->recv_buffer[client->recv_buffer_size],
						  sizeof (client->recv_buffer) - client->recv_buffer_size,
						  NULL, &error);

  if (res < 0 && g_error_matches (error, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK))
    {
      g_error_free (error);
      res = 0;
    }
  else if (res <= 0)
    {
      g_printerr ("Unable to read from client connection: %s\n", error ? error->message : "eof");
      exit (1);
    }

  client->recv_buffer_size += res;
}

// For a given client, process all the extracted messages
// in its input queue.
static void
process_incoming_messages (client_t *client)
{
  GList *first;

  while (g_list_first (client->incoming) != NULL)
    {
      first = g_list_first (client->incoming);
      process_message (client, first->data);
      joza_msg_free (first->data);
      g_list_remove_link (client->incoming, first);
      g_list_free_1(first);
    }
}

static void
do_message_parse_error (client_t *client, msg_t *msg)
{
  if (client->state == C1_UNINITIALIZED)
    // Don't reply.  Disconnect
  else if (client->state == C2_CLIENT_RESTART_REQUEST)
    // Drop the message.  We're restarting anyway.
  else if (client->state == C3_SERVER_RESTART_REQUEST)
    // Drop the message.  We're restarting anyway.
  else if (client->state == C4_READY)
    // Begin a server-requested restart
}


static void
process_message (client_t *client, msg_t *msg)
{
  if (client->state == C1_UNINITIALIZED)
    process_message_in_c1_unitialized_state (client_t *client, msg_t *msg);
  else if (client->state == C2_CLIENT_RESTART_REQUEST)
    process_message_in_c2_client_restart_request_state (client_t *client, msg_t *msg);
  else if (client->state == C3_SERVER_RESTART_REQUEST)
    process_message_in_c3_server_restart_request_state (client_t *client, msg_t *msg);
  else if (client->state == C4_READY)
    process_message_in_c4_ready_state (client_t *client, msg_t *msg);
}

void     process_message_in_c1_unitialized_state (client_t *client, msg_t *msg);
{
  // The only valid message for an unitialized client is CONNECT
  if (msg->id == JZ_MSG_CONNECT) {
    // Check if this calling address is in use.
    GList *l;
    gboolean name_in_use = FALSE;
    for (l = client_list; l != NULL; l = l->next)
      {
        if (g_str_equal (l->data->address, msg->called_address))
          {
            name_in_use = TRUE;
            brak;
          }
      }
    // If it is, send a DIAGNOSTIC
    if (name_in_use)
      {
        JzMsg *msg = jz_msg_new_diagnostic (CAUSE, DIAGNOSTIC);
        jz_client_send_msg (client, msg);
        jz_msg_free (msg);
      }
    else
      {
        JzMsg *msg = jz_msg_new_connect_indication ();
        jz_client_send_msg (client, msg);
        jz_msg_free (msg);
        client->state == C4_INITIALIZED;
        client->address = g_strdup (msg->called_address);
        client->hostname = g_strdup (msg->hostname);
        jz_client_update_modified_time (client);
      }
  }
  else
    // Ignore the message
    ;
}

void     process_message_in_c2_client_restart_request_state (client_t *client, msg_t *msg)
{
  // WTF.  The client has requested a restart and it is still sending messages?
  // Punish.
}

void     process_message_in_c3_server_restart_request_state (client_t *client, msg_t *msg)
{
  // The only valid message in this state is a RESTART_CONFIRMATION
}

void     process_message_in_c4_ready_state (client_t *client, msg_t *msg)
{
  // This client has registered its name, so CONNECT is invalid.
  if (msg->id == JZ_MSG_CONNECT)
    {
      // Send diagnostic and drop message
    }
  else if (msg->id == JZ_MSG_DISCONNECT)
    {
      // Send a DISCONNECT
      // return to unitialized state
      // start the CONNECT timer
    }
  else if (msg->id == JZ_MSG_CALL_REQUEST)
    {
      // We handle that: Initiate new call
    }
  else if (msg->is == JZ_MSG_CALL_ACCEPTED)
    {
      // We handle that: Complete new call
    }
  else if (msg_is (JZ_MSG_DATA, JZ_MSG_RR, JZ_MSG_RNR, JZ_MSG_CLEAR_REQUEST,
                   JZ_MSG_CLEAR_CONFIRMATION, JZ_MSG_RESET_REQUEST, JZ_MSG_RESET_CONFIRMATION))
    {
      // We ask the appropriate channel to handle these
      JzChannel *channel = g_hash_table_lookup (msg->called_address);
      jz_channel_process_message (channel, msg);
    }
}

static gboolean
client_input_available_cb (gpointer stream, gpointer user_data)
{
  client_t *client = user_data;

  read_some_input_nonblocking (client);
  parse_all_input (client);

  process_input_messages (client);

  return G_SOURCE_CONTINUE;
}

void _g_socket_connection_attach_input_callback (GSocketConnection *connection,
                                                 GSource **source,
                                                 GSourceFunc callback,
                                                 gpointer user_data)
{
  GInputStream *in;
  GPollableInputStream *pollable;

  in = g_io_stream_get_input_stream (G_IO_STREAM (connection));
  pollable = G_POLLABLE_INPUT_STREAM (in);
  *source = g_pollable_input_stream_create_source (pollable, NULL);
  g_source_attach (source, NULL);
  g_source_set_callback (source, callback, user_data, NULL);
}



void client_init(GSocketConnection *connection)
{
  client_t *client = g_new0(client_t, 1);

  g_message ("In client init");
  GSocketAddress *remote =
    g_socket_connection_get_remote_address(connection, NULL);
  GSocketAddress *local =
    g_socket_connection_get_local_address(connection, NULL);

  char *remote_address_str = g_inet_address_to_string (g_inet_socket_address_get_address (G_INET_SOCKET_ADDRESS (remote)));
  guint16 remote_port = g_inet_socket_address_get_port (G_INET_SOCKET_ADDRESS (remote));
  char *local_address_str = g_inet_address_to_string (g_inet_socket_address_get_address (G_INET_SOCKET_ADDRESS (local)));
  guint16 local_port = g_inet_socket_address_get_port (G_INET_SOCKET_ADDRESS (local));
  g_message ("incoming connection %s:%u to %s:%u\n", remote_address_str, remote_port, local_address_str, local_port);
  g_free (remote_address_str);

  client->connection = connection;
  client->address_str = remote_address_str;
  client->port = remote_port;
  client->key = s_client_hash(remote_address_str, remote_port);
  client->state = C1_READY;

  // A connection has input and output ports.  Here, I take the
  // input port and hook it into the main loop, so that it calls
  // a callback function whenever there is input available on the
  // input port.

  _g_socket_connection_attach_input_callback (client->connection,
                                              &(client->input_stream_source),
                                              client_input_available_cb,
                                              client);
}


#endif
