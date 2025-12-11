#include "string_uint.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

int readstring(int fd, struct string *sbuf) {
    if (read(fd, &(sbuf->length), sizeof(uint32_t)) < 0) return 1;

    sbuf->data = malloc(be32toh((sbuf->length) + 1) * sizeof(uint8_t));
    if(!sbuf->data) return 1;

    if (read(fd, sbuf->data, be32toh(sbuf->length) * sizeof(uint8_t)) < 0) return 1;
    sbuf->data[be32toh(sbuf->length)] ='\0';
    return 0;
}

int writestring(int fd, struct string *sbuf) {
    if (write(fd, &(sbuf->length), sizeof(uint32_t)) < 0) return 1;
    if (write(fd, sbuf->data, be32toh(sbuf->length) * sizeof(uint8_t)) < 0) return 1;
    return 0;
}

void freestring(struct string *sbuf) {
    free(sbuf->data);
}