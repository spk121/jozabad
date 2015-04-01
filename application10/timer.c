// A modified version of glib timers that need the following properties.

// After a channel in state P3_SERVER_WAITING sends out the INCOMING
// CALL to a client, it waits for T11=180 seconds for a CALL ACCEPTED
// response or a CLEAR REQUEST response from the client.

// On timeout, it issues a punishment_clear_request of local_procecure_error
// and time_expired_for_incoming_call diagnostic

void jz_timer_new (JzChannel *X, int duration, time_type_t type, int ID, int iterations)
{
	
}
