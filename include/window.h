/*
 * File:   parch_window.h
 * Author: mike
 *
 * Created on August 21, 2013, 8:25 PM
 */

#ifndef PARCH_WINDOW_H
#define	PARCH_WINDOW_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#define WINDOW_MIN 1U
#define WINDOW_MAX 32768U
#define WINDOW_DEFAULT 2U
#define WINDOW_NOMINAL 2U

bool
parch_window_validate(uint16_t i);
uint16_t
parch_window_apply_default(uint16_t i);
uint16_t
parch_window_throttle(uint16_t request, uint16_t limit);
bool
parch_window_negotiate(uint16_t request, uint16_t current);

#ifdef	__cplusplus
}
#endif

#endif	/* PARCH_WINDOW_H */

