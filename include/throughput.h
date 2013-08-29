/*
 * File:   parch_throughput.h
 * Author: mike
 *
 * Created on August 21, 2013, 5:04 PM
 */

#ifndef PARCH_THROUGHPUT_H
#define	PARCH_THROUGHPUT_H

#include <stdbool.h>
#include <assert.h>
#include <stdint.h>

#define THROUGHPUT_NAME_MAX_LEN (12)

typedef enum _throughput_t {
    t_unspecified,
    t_reserved_1,
    t_reserved_2,
    t_75bps,
    t_150bps,
    t_300bps,
    t_600bps,
    t_1200bps,
    t_2400bps,
    t_4800bps,
    t_9600bps,
    t_19200bps,
    t_48kpbs,
    t_64kbps,
    t_128kbps,
    t_192kbps,
    t_256kbps,
    t_320kbps,
    t_384kbps,
    t_448kbps,
    t_512kbps,
    t_578kbps,
    t_640kbps,
    t_704kbps,
    t_768kbps,
    t_832kbps,
    t_896kbps,
    t_960kbps,
    t_1024kbps,
    t_1088kbps,
    t_1152kbps,
    t_1216kbps,
    t_1280kbps,
    t_1344kbps,
    t_1408kbps,
    t_1472kbps,
    t_1536kbps,
    t_1600kbps,
    t_1664kbps,
    t_1728kbps,
    t_1792kbps,
    t_1856kbps,
    t_1920kbps,
    t_1984kbps,
    t_2048kbps,

    t_default = t_64kbps,
    t_modem_upload = t_19200bps,
    t_dsl_upload_tier_1 = t_384kbps,
    t_dsl_upload_tier_2 = t_768kbps,
    t_last = t_2048kbps
} throughput_t;
    
extern const char throughput_names[t_last + 1][THROUGHPUT_NAME_MAX_LEN + 1];
extern const uint32_t throughput_bps[t_last + 1];

char const * const
name(throughput_t i);
bool
    validate(throughput_t i);
throughput_t
    apply_default(throughput_t i);
throughput_t
    throttle(throughput_t request, throughput_t limit);
bool
    negotiate(throughput_t request, throughput_t current);
uint32_t
    bps(uint8_t i);

#endif	/* PARCH_THROUGHPUT_H */

