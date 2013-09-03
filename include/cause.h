/*
 * File:   cause.h
 * Author: mike
 *
 * Created on September 1, 2013, 10:52 AM
 */

#ifndef CAUSE_H
#define	CAUSE_H

#define CAUSE_NAME_MAX_LEN (40)

typedef enum _cause_t {
    c_worker_originated,
    c_number_busy,
    c_not_obtainable,
    c_out_of_order,
    c_local_procedure_error,
    c_remote_procedure_error,
    c_invalid_facility_request,
    c_access_barred,
    c_network_congestion,

    c_last = c_network_congestion
} cause_t;

extern const char cause_names[c_last + 1][CAUSE_NAME_MAX_LEN + 1];

char const *
name(cause_t a);

#endif	/* CAUSE_H */

