// -*- c-basic-offset: 4; c-backslash-column: 79; indent-tabs-mode: nil -*-
// vim:sw=4 ts=4 sts=4 expandtab
#ifndef TIMEVAL_H_100409
#define TIMEVAL_H_100409
#include <stdint.h>
#include <sys/time.h>
#include <stdbool.h>
#include <limits.h>

/** @file
 * @brief utilities for handling struct timeval
 */

/// Define a struct timeval
#define TIMEVAL_INITIALIZER { 0, 0 }
#define END_OF_TIME { LONG_MAX, LONG_MAX }

/// @return microseconds
int64_t timeval_sub(struct timeval const *restrict, struct timeval const *restrict);

int64_t timeval_age(struct timeval const *);

static inline bool timeval_is_set(struct timeval const *tv)
{
    return tv->tv_sec != 0;
}

// this one is unset
extern struct timeval const timeval_unset;

static inline void timeval_reset(struct timeval *tv)
{
    tv->tv_sec = 0;
}

int timeval_cmp(struct timeval const *restrict, struct timeval const *restrict);
void timeval_add_usec(struct timeval *, int64_t usec);
void timeval_add_sec(struct timeval *, int32_t sec);
void timeval_sub_usec(struct timeval *, int64_t usec);
void timeval_sub_msec(struct timeval *, int64_t msec);
void timeval_sub_sec(struct timeval *, int32_t sec);
char const *timeval_2_str(struct timeval const *);
void timeval_set_now(struct timeval *);
void timeval_set_min(struct timeval *restrict, struct timeval const *restrict);
void timeval_set_max(struct timeval *restrict, struct timeval const *restrict);
void timeval_serialize(struct timeval const *, uint8_t **);
void timeval_deserialize(struct timeval *, uint8_t const **);

#endif
