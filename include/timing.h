#ifndef TIMING_H
#define TIMING_H

#include <stdint.h>

struct timing {
    uint64_t minutes;
    uint32_t hours;
    uint8_t daysofweek;
};

int readtiming(int fd, struct timing *tbuf);
int is_it_time(struct timing *tbuf);

#endif