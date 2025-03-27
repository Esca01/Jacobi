#include "timing.h"

long double timespec_to_dbl(struct timespec x)
{
    return (long double)x.tv_sec + (long double)x.tv_nsec * 1.0e-9;
}
long double timespec_diff(struct timespec start, struct timespec finish)
{
    return (finish.tv_sec - start.tv_sec) + (finish.tv_nsec - start.tv_nsec) * 1e-9;
}

void get_time(struct timespec *t)
{
    clock_gettime(CLOCK_MONOTONIC, t);
}
