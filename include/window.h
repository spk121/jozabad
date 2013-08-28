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

#define WINDOW_MIN 1
#define WINDOW_MAX 32768
#define WINDOW_DEFAULT 2
#define WINDOW_NOMINAL 2

typedef struct _window_negotiation {
    bool ok;
    uint32_t size;
} parch_window_negotiation_t;

bool
parch_window_validate(uint32_t i);
uint32_t
parch_window_apply_default(uint32_t i);
uint32_t
parch_window_throttle(uint32_t request, uint32_t limit);
parch_window_negotiation_t
parch_window_negotiate(uint32_t request, uint32_t current);

#ifdef	__cplusplus
}
#endif

#endif	/* PARCH_WINDOW_H */

