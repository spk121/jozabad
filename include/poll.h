/*
 * File:   poll.h
 * Author: mike
 *
 * Created on August 23, 2013, 6:18 AM
 */

#ifndef POLL_H_INCLUDED
#define	POLL_H_INCLUDED

#ifdef	__cplusplus
extern "C" {
#endif

extern void *sock;

void
poll_init(bool verbose, const char *endpoint);
void
poll_start (void);
#if 0
void add_timer(size_t delay_in_sec, zloop_fn handler, void *arg);
void send_msg_and_free(parch_msg_t **ppmsg);

void poll_start (void);
#endif


#ifdef	__cplusplus
}
#endif

#endif	/* POLL_H */

