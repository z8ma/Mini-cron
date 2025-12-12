#include "string_uint.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

int readstring(int fd, struct string *sbuf) {

    uint32_t length_be;

    if (read(fd, &length_be, sizeof(uint32_t)) < 0) return 1;
    uint32_t host_length = be32toh(length_be);
    sbuf->length = host_length;

    sbuf->data = malloc((host_length + 1) * sizeof(uint8_t));
    if(!sbuf->data) return 1;

    if (read(fd, sbuf->data, host_length * sizeof(uint8_t)) != (ssize_t)host_length * sizeof(uint8_t)) return 1;
    sbuf->data[host_length] ='\0';
    return 0;
}

int writestring(int fd, struct string *sbuf) {
    uint32_t length_be = htobe32(sbuf->length);
    if (write(fd, &length_be, sizeof(uint32_t)) != sizeof(uint32_t)) return 1;

    if (write(fd, sbuf->data,sbuf->length * sizeof(uint8_t)) != (ssize_t)sbuf->length * sizeof(uint8_t)) return 1;
    return 0;
}

void freestring(struct string *sbuf) {
    free(sbuf->data);
}