/*
 * Macros for time keeping
 *
 * Lorenz Gerber, October 2017,
 * modified from Pacheco (2011, Parallel Programming)
 *
 * Args should be double, not pointer to double.
 */
#ifndef _TIMER_H_
#define _TIMER_H_

#include <sys/time.h>
#include <time.h>

/* The arguments now should be a double (not a pointer to a double) */
#define GET_WALL_TIME(now) { \
    struct timeval t; \
    gettimeofday(&t, NULL); \
    now = t.tv_sec + t.tv_usec/1000000.0; \
  }

#define GET_CPU_THREAD_TIME(now){ \
  struct timespec ts; \
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts); \
  now = (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000; \
}

#endif
