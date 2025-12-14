#ifndef TIMES_EXITCODES_H
#define TIMES_EXITCODES_H

#include <stdint.h>
#include <stdlib.h>
#include "string_uint.h"

struct run {
    uint64_t time;
    uint16_t exitcode;
};

struct times_exitcodes{
    uint32_t nbruns;
    struct run *runs;
};

int readrun(int fd, struct run *rbuf);
int writerun(int fd, struct run *rbuf);
int read_times_exitcodes(int fd, struct times_exitcodes *tec);
int write_times_exitcodes(int fd, struct times_exitcodes *tec);
int run_to_string(struct run r, struct string *s);
int times_exitcodes_to_string(struct times_exitcodes tec, struct string *s);

#endif