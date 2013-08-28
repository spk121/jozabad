/*
 * File:   parch_direction.h
 * Author: mike
 *
 * Created on August 21, 2013, 8:25 PM
 */

#ifndef PARCH_DIRECTION_H
#define	PARCH_DIRECTION_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stdbool.h>

// Note that bit 0 is input barred and bit 1 is output barred
enum _direction_t {
    direction_bidirectional = 0,
    direction_input_barred = 1,
    direction_output_barred = 2,
    direction_io_barred = 3,
};

typedef enum _direction_t direction_t;

#define DIRECTION_DEFAULT direction_default

bool
parch_direction_validate(direction_t i);
direction_t
parch_direction_throttle(direction_t request, direction_t limit);
bool
parch_direction_negotiate(direction_t request, direction_t current);

#ifdef	__cplusplus
}
#endif

#endif	/* PARCH_DIRECTION_H */

