/*
 * File:   poll.h
 * Author: mike
 *
 * Created on August 23, 2013, 6:18 AM
 */

#ifndef POLL_H_INCLUDED
#define	POLL_H_INCLUDED
#include "lib.h"

void poll_init(bool_t verbose, const char *endpoint);
void poll_start (void);
#endif

