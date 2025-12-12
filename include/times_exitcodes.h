#ifndef TIMES_EXITCODES_H
#define TIMES_EXITCODES_H

#include <stdint.h>
#include <stdlib.h>

struct times_exitcodes{
    uint64_t time;
    uint16_t exitcode;
};

typedef struct times_exitcodes_list {
    times_exitcodes *records; 
    uint32_t count;              
} times_exitcodes_list_t;

int read_times_exitcodes(int fd, struct times_exitcodes *tec, size_t n);
int write_times_exitcodes(int fd, struct times_exitcodes *tec, size_t n);
int readtec(int fd, struct times_exitcodes *tec);

#endif