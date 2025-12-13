#include "times_exitcodes.h"

#include <stdlib.h>
#include <unistd.h>

int read_times_exitcodes(int fd, struct times_exitcodes *tec, size_t n){
    uint64_t time_be;
    uint16_t exitcode_be;
    
    if (read(fd, &time_be, sizeof(uint64_t)) != sizeof(uint64_t)) {
        return -1; 
    }
   
    tec->time = be64toh(time_be);

    if (read(fd, &exitcode_be, sizeof(uint16_t)) != sizeof(uint16_t)) {
        return -1; 
    }
    tec->exitcode = be16toh(exitcode_be);
    (void)n;
    return 0;
}

int write_times_exitcodes(int fd, struct times_exitcodes *tec, size_t n){
    uint64_t time_be = htobe64(tec->time);
    if (write(fd, &time_be, sizeof(uint64_t)) != sizeof(uint64_t)) {
        return -1; 
    }
    uint16_t exitcode_be = htobe16(tec->exitcode);
    if (write(fd, &exitcode_be, sizeof(uint16_t)) != sizeof(uint16_t)) {
        return -1;
    }
    (void)n;
    return 0;
}


int readtec(int fd, struct times_exitcodes *tec) {
    uint64_t time_be;
    uint16_t exitcode_be;

    if (read(fd, &time_be, sizeof(int64_t)) != sizeof(int64_t)) { return 1; }
    tec->time = be64toh(time_be);

    
    if (read(fd, &exitcode_be, sizeof(uint16_t)) != sizeof(uint16_t)) { return 1; }
    tec->exitcode = be16toh(exitcode_be);

    return 0;
}