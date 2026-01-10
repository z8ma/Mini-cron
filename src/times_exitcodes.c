#include "times_exitcodes.h"
#include "string_uint.h"

#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

int readrun(int fd, struct run *rbuf) {
    uint64_t time_be;
    read(fd, &time_be, sizeof(uint64_t));
    rbuf->time = be64toh(time_be);

    uint16_t exit_code_be;
    read(fd, &exit_code_be, sizeof(uint16_t));
    rbuf->exitcode = be16toh(exit_code_be);
    return 0;
}

int writerun(int fd, struct run *rbuf) {
    uint64_t time_be = htobe64(rbuf->time);
    write(fd, &time_be, sizeof(uint64_t));

    uint16_t exit_code_be = htobe16(rbuf->exitcode);
    write(fd, &exit_code_be, sizeof(uint16_t));
    return 0;
}

int read_times_exitcodes(int fd, struct times_exitcodes *tec){
    if (tec->nbruns == 0) {
        struct stat st;
        if (fstat(fd, &st) < 0) return 1;
        tec->nbruns = st.st_size / (sizeof(uint16_t)+ sizeof(uint64_t));
    }
    tec->runs = malloc(tec->nbruns * sizeof(struct run));
    for (int i = 0; i < tec->nbruns; i++) {
        readrun(fd, (tec->runs+i));
    }
    return 0;
}

int write_times_exitcodes(int fd, struct times_exitcodes *tec){
    uint32_t nbruns_be = htobe32(tec->nbruns);
    write(fd, &nbruns_be, sizeof(uint32_t));
    for (int i = 0; i < tec->nbruns; i++) {
        writerun(fd, (tec->runs+i));
    }
    return 0;
}

int run_to_string(struct run r, struct string *s) {
    struct string dash  = {1, (uint8_t *)"-"};
    struct string space = {1, (uint8_t *)" "};
    struct string colon = {1, (uint8_t *)":"};
    struct tm *tm = localtime((time_t*)&r.time);

    uint_to_string((uint64_t)(tm->tm_year + 1900), s);
    catstring(s, dash);
    if (tm->tm_mon < 10) uint_to_string(0, s);
    uint_to_string((uint64_t)(tm->tm_mon + 1), s);
    catstring(s, dash);
    if (tm->tm_mday < 10) uint_to_string(0, s);
    uint_to_string((uint64_t)tm->tm_mday, s);
    catstring(s, space);

    if (tm->tm_hour < 10) uint_to_string(0, s);
    uint_to_string((uint64_t)tm->tm_hour, s);
    catstring(s, colon);

    if (tm->tm_min < 10) uint_to_string(0, s);
    uint_to_string((uint64_t)tm->tm_min, s);
    catstring(s, colon);

    if (tm->tm_sec < 10) uint_to_string(0, s);
    uint_to_string((uint64_t)tm->tm_sec, s);

    catstring(s, space);
    uint_to_string((uint64_t)r.exitcode, s);

    return 0;
}


int times_exitcodes_to_string(struct times_exitcodes tec, struct string *s) {
    struct string rl = {1, (uint8_t *)"\n"};
    for (int i = 0; i < tec.nbruns; i++) {
        run_to_string(tec.runs[i], s);
        catstring(s, rl);
    }
    return 0;
}