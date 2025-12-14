#ifndef STRING_H
#define STRING_H

#include <stdint.h>


struct string {
    uint32_t length;
    uint8_t *data;

};

int readstring(int fd, struct string *sbuf);
int writestring(int fd, struct string *sbuf);
void freestring(struct string *sbuf);
int catstring(struct string *newstring, struct string oldstring);
int uint_to_string(uint64_t n, struct string *s);

#endif