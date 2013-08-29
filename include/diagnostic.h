/*
 * File:   parch_diagnostic.h
 * Author: mike
 *
 * Created on August 21, 2013, 7:54 PM
 */

#ifndef PARCH_DIAGNOSTIC_H
#define	PARCH_DIAGNOSTIC_H

#define DIAGNOSTIC_NAME_MAX_LEN (40)

typedef enum _diagnostic_t {
    d_unknown,
    d_action_invalid,
    d_data_packet_not_in_window,
    d_data_packet_out_of_order,
    d_data_packet_too_large,
    d_data_packet_too_small,
    d_direction_invalid,
    d_direction_invalid_negotiation,
    d_packet_index_invalid_negotiation,
    d_packet_index_too_large,
    d_packet_index_too_small,
    d_state_invalid,
    d_throughput_index_invalid_negotiation,
    d_throughput_index_too_large,
    d_throughput_index_too_small,
    d_window_edge_out_of_range,
    d_window_invalid_negotiation,
    d_window_too_large,
    d_window_too_small,
    d_last = d_window_too_small,
} diagnostic_t;

extern const char diagnostic_names[d_last + 1][DIAGNOSTIC_NAME_MAX_LEN + 1];

extern diagnostic_t diagnostic;

char const * const
    name(diagnostic_t a);

#endif	/* PARCH_DIAGNOSTIC_H */

