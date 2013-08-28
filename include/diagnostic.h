/*
 * File:   parch_diagnostic.h
 * Author: mike
 *
 * Created on August 21, 2013, 7:54 PM
 */

#ifndef PARCH_DIAGNOSTIC_H
#define	PARCH_DIAGNOSTIC_H

#ifdef	__cplusplus
extern "C" {
#endif

enum _diagnostic_t {
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
};

typedef enum _diagnostic_t diagnostic_t;

extern diagnostic_t diagnostic;

#ifdef	__cplusplus
}
#endif

#endif	/* PARCH_DIAGNOSTIC_H */

