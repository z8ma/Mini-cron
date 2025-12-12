#include "timing.h"

#include <stdint.h>
#include <endian.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>


int readtiming(int fd, struct timing *tbuf) {
    
    if (read(fd, &(tbuf->minutes), sizeof(uint64_t)) != sizeof(uint64_t)) return 1;
    tbuf->minutes = be64toh(tbuf->minutes); 

    
    if (read(fd, &(tbuf->hours), sizeof(uint32_t)) != sizeof(uint32_t)) return 1;
    tbuf->hours = be32toh(tbuf->hours); 

    
    if (read(fd, &(tbuf->daysofweek), sizeof(uint8_t)) != sizeof(uint8_t)) return 1;
    return 0;
}

int writetiming(int fd, struct timing *tbuf) {
    uint64_t minutes_be = htobe64(tbuf->minutes);
    if (write(fd, &minutes_be, sizeof(uint64_t)) != sizeof(uint64_t)) return 1;

    uint32_t hours_be = htobe32(tbuf->hours);
    if (write(fd, &hours_be, sizeof(uint32_t)) != sizeof(uint32_t)) return 1;

    if (write(fd, &(tbuf->daysofweek), sizeof(uint8_t)) != sizeof(uint8_t)) return 1;
    return 0;
}

int is_it_time(struct timing *tbuf) {
    time_t timestamp = time(NULL);
    struct tm *now = localtime(&timestamp);
    if ((tbuf->daysofweek & (1<<(now->tm_wday))) == 0) {
        return 0;
    } else if ((be32toh(tbuf->hours) & (1<<(now->tm_hour))) == 0) {
        return 0;
    } else if ((be64toh(tbuf->minutes) & (1ULL<<(now->tm_min))) == 0) {
       return 0; 
    }
    return 1;
}