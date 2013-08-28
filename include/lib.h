/*
 * File:   lib.h
 * Author: mike
 *
 * Created on August 17, 2013, 12:36 PM
 */

#ifndef _PARCH_LIB_H_INCLUDE
#define	_PARCH_LIB_H_INCLUDE

#ifdef	__cplusplus
extern "C" {
#endif

zctx_t *
zctx_new_or_die (void);
void *
zsocket_new_or_die(zctx_t *ctx, int type);
zloop_t *
zloop_new_or_die(void);
bool
is_safe_ascii(const char * const str) __attribute__((pure, nonnull(1)));

#ifdef	__cplusplus
}
#endif

#endif	/* LIB_H */

