#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include "serialisation.h"

int readstring(int fd, struct string *sbuf) {
    if (read(fd, sbuf, sizeof(uint32_t)) < 0) return 1;

    sbuf->length = be32toh(sbuf->length);
    sbuf->data = malloc(((sbuf->length) + 1) * sizeof(uint8_t));
    if(!sbuf->data) return 1;

    if (read(fd, sbuf->data, (sbuf->length) * sizeof(uint8_t)) < 0) return 1;
    sbuf->data[sbuf->length] ='\0';
    return 0;
}

void freestring(struct string *sbuf) {
    free(sbuf->data);
}

int readtiming(int fd, struct timing *tbuf) {
    if (read(fd, tbuf, sizeof(struct timing)) < 0) return 1;

    tbuf->minutes = be64toh(tbuf->minutes);
    tbuf->hours = be32toh(tbuf->hours);
    return 0;
}