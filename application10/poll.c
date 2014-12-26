#include <gtk/gtk.h>
#include <czmq.h>

#include "exampleapp.h"
#include "exampleappwin.h"

zctx_t *z_context = NULL;
void *z_socket = NULL;
zmq_pollitem_t z_items[1];

void
z_poll_init()
{
  z_context = zctx_new();
  z_socket = zsocket_new(z_context, ZMQ_ROUTER);
  zsocket_bind(z_socket, "tcp://*:8516");
  z_items[0].socket = z_socket;
  z_items[0].fd = 0;
  z_items[0].events = ZMQ_POLLIN;
  z_items[0].revents = 0;
}

gboolean
z_poll_iterate(void *dummy)
{
  int i;
  int rc;
  zmq_msg_t msg;

  rc = zmq_poll (z_items, 1, 0);  
  if (rc < 0)
    {
      g_message("poll failure");
      return G_SOURCE_REMOVE;
    }
  if (rc > 0)
    {
      g_message("poll received %d messages", rc);
      for (i = 0; i < rc; i ++) {
        zmq_msg_init (&msg);
        zmq_recvmsg (z_socket, &msg, 0);

        switch (poll_get_msg_type (msg))
          {
          case CONNECT:
            poll_process_connect_msg (msg);
            break;
          case DISCONNECT:
            poll_process_disconnect_msg (msg);
            break;
          default:
            break;
          }
        zmq_msg_close (&msg);
      }
    }
  return G_SOURCE_CONTINUE;

}
