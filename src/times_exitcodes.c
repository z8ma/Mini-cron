#include "times_exitcodes.h"

#include <stdlib.h>
#include <unistd.h>

int read_times_exitcodes(int fd, struct times_exitcodes *tec, size_t n){
    read(fd, &(tec->time), sizeof(uint64_t));
    read(fd, &(tec->exitcode), sizeof(uint16_t));
    return 0;
}

int write_times_exitcodes(int fd, struct times_exitcodes *tec, size_t n){
    write(fd, &(tec->time), sizeof(uint64_t));
    write(fd, &(tec->exitcode), sizeof(uint16_t));
    return 0;
}


int readtec(int fd, struct times_exitcodes *tec) {
   
}