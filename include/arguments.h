#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include "string_uint.h"
#include <stdint.h>


struct arguments {
    uint32_t argc;
    struct string *argv;
};

int readarguments(int fd, struct arguments *abuf);
int writearguments(int fd, struct arguments *abuf);
void freearguments(struct arguments *abuf);
uint16_t executearg(struct arguments *abuf);

#endif