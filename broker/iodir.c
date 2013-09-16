#include "iodir.h"

int iodir_validate(iodir_t x)
{
	if (x == io_bidirectional
		|| x == io_incoming_calls_barred
		|| x == io_outgoing_calls_barred)
		return 1;
	return 0;
}