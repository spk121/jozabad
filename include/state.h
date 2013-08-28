/*
 * File:   parch_state.h
 * Author: mike
 *
 * Created on August 21, 2013, 8:25 PM
 */

#ifndef PARCH_STATE_H
#define	PARCH_STATE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>

enum _state_t {
    state_ready,
    state_x_call_request,
    state_y_call_request,
    state_data_transfer,
    state_call_collision,
    state_x_clear_request,
    state_y_clear_request,
    state_x_reset_request,
    state_y_reset_request,

    state_min = state_ready,
    state_max = state_y_reset_request,
};

typedef enum _state_t state_t;

extern const char state_names[][25];

bool
parch_state_validate(state_t i);

#ifdef	__cplusplus
}
#endif

#endif	/* PARCH_STATE_H */

