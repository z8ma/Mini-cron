#include "timing.h"

#include <stdint.h>
#include <endian.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>


int readtiming(int fd, struct timing *tbuf) {
    if (read(fd, tbuf, sizeof(struct timing)) < 0) return 1;
    return 0;
}

int writetiming(int fd, struct timing *tbuf) {
    if (write(fd, tbuf, sizeof(struct timing)) < 0) return 1;
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