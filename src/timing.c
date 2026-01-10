#include "timing.h"
#include <string_uint.h>

#include <stdint.h>
#include <endian.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <stdio.h>


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
    } else if ((tbuf->hours & (1<<(now->tm_hour))) == 0) {
        return 0;
    } else if ((tbuf->minutes & (1ULL<<(now->tm_min))) == 0) {
       return 0; 
    }
    return 1;
}

int timing_to_string(struct timing t, struct string *s) {
    struct string comma = {1, (uint8_t*)","};
    struct string space = {1, (uint8_t*)" "};
    struct string star = {1, (uint8_t*)"*"};
    struct string dash = {1, (uint8_t*)"-"};
    int first;
    int pred;
    int duration;

    if (t.minutes == 0) {
        catstring(s, dash);
    } else if ((t.minutes & 0xfffffffffffffff) == 0xfffffffffffffff) {
        catstring(s, star);
    } else {
        first = 1;
        pred = -1;
        duration = 0;
        for (int i = 0; i < 60; i++) {
            if (t.minutes & (1ULL << i)) {
                if (!first && i != 59) {
                    if (pred != i - 1) {
                        if (duration) {
                            uint_to_string(pred, s);
                            duration = 0;
                        }
                        catstring(s, comma);
                    } else if(!duration) {
                        duration = 1;
                        catstring(s, dash);
                    }
                }
                if (!duration || i == 59) {
                    uint_to_string(i, s);
                }
                first = 0;
                pred = i;
            } else if (duration) {
                duration = 0;
                uint_to_string(pred, s);
            }
        }
    }
    catstring(s, space);
    if (t.hours == 0) {
        catstring(s, dash);
    } else if ((t.hours & 0xffffff) == 0xffffff) {
        catstring(s, star);
    } else {
        first = 1;
        pred = -1;
        duration = 0;
        for (int i = 0; i < 24; i++) {
            if (t.hours & (1u << i)) {
                    if (!first && i != 23) {
                    if (pred != i - 1) {
                        if (duration) {
                            uint_to_string(pred, s);
                            duration = 0;
                        }
                        catstring(s, comma);
                    } else if(!duration) {
                        duration = 1;
                        catstring(s, dash);
                    }
                }
                if (!duration || i == 23) {
                    uint_to_string(i, s);
                }
                first = 0;
                pred = i;
            } else if (duration) {
                duration = 0;
                uint_to_string(pred, s);
            }
        }
    }
    catstring(s, space);

    if (t.daysofweek == 0) {
        catstring(s, dash);
    } else if ((t.daysofweek & 0x7f) == 0x7f) {
        catstring(s, star);
    } else {
        first = 1;
        pred = -1;
        duration = 0;
        for (int i = 0; i < 7; i++) {
            if (t.daysofweek & (1 << i)) {
                    if (!first && i != 6) {
                    if (pred != i - 1) {
                        if (duration) {
                            uint_to_string(pred, s);
                            duration = 0;
                        }
                        catstring(s, comma);
                    } else if(!duration) {
                        duration = 1;
                        catstring(s, dash);
                    }
                }
                if (!duration || i == 6) {
                    uint_to_string(i, s);
                }
                first = 0;
                pred = i;
            } else if (duration) {
                duration = 0;
                uint_to_string(pred, s);
            }
        }
    }
    return 0;
}

int string_to_timing(struct string minutes, struct string hours, struct string daysofweek, struct timing *t) {
    if (strcmp("*", (char *) minutes.data) == 0) {
        t->minutes = 0xfffffffffffffff;
        minutes.length = 0;
    } else if (strcmp("-", (char *) minutes.data) == 0) {
        t->minutes = 0;
        minutes.length = 0;
    }
    int pred = -1;
    int duration = 0;
    int value = -1;
    for (int i = 0; i < minutes.length; i++) {
        if (minutes.data[i] == ',') {
            if (value == -1) {
                return -1;
            }
            value = -1;
            continue;
        }
        if (minutes.data[i] == '-') {
            if (value == -1) {
                return -1;
            }
            value = -1;
            duration = 1;
            continue;
        }
        if (minutes.data[i] - '0' > 9 || minutes.data[i] - '0' < 0) {
            return minutes.data[i] + 1;
        }
        if (minutes.data[i + 1] - '0' <= 9 && minutes.data[i + 1] - '0' >= 0) {
            value = (minutes.data[i] - '0') * 10 + minutes.data[i + 1] - '0';
            i++;
        } else {
            value = minutes.data[i] - '0';
        }
        if (duration) {
            for (int j = pred + 1; j < value; j++) {
                t->minutes |= (1ULL << j);
            }
            duration = 0;
        }
        t->minutes = t->minutes | (1ULL << value);
        pred = value;
    }


    if (strcmp("*", (char *) hours.data) == 0) {
        t->hours = 0xffffff;
        hours.length = 0;
    } else if (strcmp("-", (char *) hours.data) == 0) {
        t->hours = 0;
        hours.length = 0;
    }
    pred = -1;
    duration = 0;
    value = -1;
    for (int i = 0; i < hours.length; i++) {
        if (hours.data[i] == ',') {
            if (value == -1) {
                return -1;
            }
            value = -1;
            continue;
        }
        if (hours.data[i] == '-') {
            if (value == -1) {
                return -1;
            }
            value = -1;
            duration = 1;
            continue;
        }
        if (hours.data[i] - '0' > 9 || hours.data[i] - '0' < 0) {
            return hours.data[i] + 1;
        }
        if (hours.data[i + 1] - '0' <= 9 && hours.data[i + 1] - '0' >= 0) {
            value = (hours.data[i] - '0') * 10 + hours.data[i + 1] - '0';
            i++;
        } else {
            value = hours.data[i] - '0';
        }
        if (duration) {
            for (int j = pred + 1; j < value; j++) {
                t->hours |= (1 << j);
            }
            duration = 0;
        }
        t->hours = t->hours | (1 << value);
        pred = value;
    }
    

    if (strcmp("*", (char *) daysofweek.data) == 0) {
        t->daysofweek = 0xff;
        daysofweek.length = 0;
    } else if (strcmp("-", (char *) daysofweek.data) == 0) {
        t->daysofweek = 0;
        daysofweek.length = 0;
    }
    pred = -1;
    duration = 0;
    value = -1;
    for (int i = 0; i < daysofweek.length; i++) {
        if (daysofweek.data[i] == ',') {
            if (value == -1) {
                return -1;
            }
            value = -1;
            continue;
        }
        if (daysofweek.data[i] == '-') {
            if (value == -1) {
                return -1;
            }
            value = -1;
            duration = 1;
            continue;
        }
        if (daysofweek.data[i] - '0' > 9 || daysofweek.data[i] - '0' < 0) {
            return daysofweek.data[i] + 1;
        }
        if (daysofweek.data[i + 1] - '0' <= 9 && daysofweek.data[i + 1] - '0' >= 0) {
            value = (daysofweek.data[i] - '0') * 10 + daysofweek.data[i + 1] - '0';
            i++;
        } else {
            value = daysofweek.data[i] - '0';
        }
        if (duration) {
            for (int j = pred + 1; j < value; j++) {
                t->daysofweek |= (1 << j);
            }
            duration = 0;
        }
        t->daysofweek = t->daysofweek | (1 << value);
        pred = value;
    }
    return 0;
}