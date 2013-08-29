/*
 * File:   parch_flow.h
 * Author: mike
 *
 * Created on August 21, 2013, 7:54 PM
 */

#ifndef PARCH_FLOW_H
#define	PARCH_FLOW_H

struct _flow_t {
    uint16_t x_send_sequence;
    uint16_t y_send_sequence;
    uint16_t x_lower_window_edge;
    uint16_t y_lower_window_edge;
    uint16_t window_size;
};

typedef struct _flow_t flow_t;

#define PACKET_SEQUENCE_MIN 0
#define PACKET_SEQUENCE_MAX 32767
#define PACKET_SEQUENCE_MODULO 32768

flow_t
reset(flow_t f);
flow_t
init(void);
bool
flow_sequence_in_range(uint16_t sequence, uint16_t lower_window_edge, uint16_t window_size);

#endif	/* PARCH_FLOW_H */

