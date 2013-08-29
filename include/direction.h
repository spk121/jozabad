/*
 * File:   parch_direction.h
 * Author: mike
 *
 * Created on August 21, 2013, 8:25 PM
 */

#ifndef PARCH_DIRECTION_H
#define	PARCH_DIRECTION_H

#define DIRECTION_NAME_MAX_LEN 14

// Note that bit 0 is input barred and bit 1 is output barred
typedef enum _direction_t {
    direction_bidirectional = 0,
    direction_input_barred = 1,
    direction_output_barred = 2,
    direction_io_barred = 3,

    direction_default = direction_bidirectional,
    direction_last = direction_io_barred
} direction_t;

typedef enum _direction_t direction_t;

char const *
name (direction_t i);
bool
validate(direction_t i);
direction_t
throttle(direction_t request, direction_t limit);
bool
negotiate(direction_t request, direction_t current);

#endif	/* PARCH_DIRECTION_H */

